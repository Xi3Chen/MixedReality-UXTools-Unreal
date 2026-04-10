#include "UxtStaticHandPoseUtilities.h"

#include "HeadMountedDisplayTypes.h"

namespace
{
	FVector GetJointWorldPosition(const FUxtTrackedHandFrame& HandFrame, EHandKeypoint Joint)
	{
		FTransform Transform;
		return HandFrame.GetJointWorldTransform(Joint, Transform) ? Transform.GetLocation() : FVector::ZeroVector;
	}

	FQuat MirrorRotationByYZPlane(const FQuat& InRotation)
	{
		const FMatrix RotationMatrix = FQuatRotationMatrix(InRotation);
		const FMatrix MirrorMatrix(
			FPlane(-1.0f, 0.0f, 0.0f, 0.0f),
			FPlane(0.0f, 1.0f, 0.0f, 0.0f),
			FPlane(0.0f, 0.0f, 1.0f, 0.0f),
			FPlane(0.0f, 0.0f, 0.0f, 1.0f));

		return FQuat(MirrorMatrix * RotationMatrix * MirrorMatrix);
	}
}

EUxtPalmCameraDirection FUxtPalmDirectionUtility::GetPalmDirectionBit(
	const FUxtTrackedHandFrame& HandFrame,
	const FTransform& CameraTransform)
{
	const FVector CameraForward = CameraTransform.GetUnitAxis(EAxis::X);
	const FVector CameraRight = CameraTransform.GetUnitAxis(EAxis::Y);
	const FVector CameraUp = CameraTransform.GetUnitAxis(EAxis::Z);

	const float ForwardDot = FVector::DotProduct(HandFrame.PalmBasis.Normal, CameraForward);
	const float HorizontalDot = FVector::DotProduct(HandFrame.PalmBasis.Normal, CameraRight);
	const float VerticalDot = FVector::DotProduct(HandFrame.PalmBasis.Normal, CameraUp);

	const float AbsForwardDot = FMath::Abs(ForwardDot);
	const float AbsHorizontalDot = FMath::Abs(HorizontalDot);
	const float AbsVerticalDot = FMath::Abs(VerticalDot);

	if (AbsForwardDot >= AbsHorizontalDot && AbsForwardDot >= AbsVerticalDot)
	{
		return ForwardDot >= 0.0f ? EUxtPalmCameraDirection::Forward : EUxtPalmCameraDirection::Backward;
	}

	if (AbsVerticalDot >= AbsHorizontalDot)
	{
		return VerticalDot >= 0.0f ? EUxtPalmCameraDirection::Up : EUxtPalmCameraDirection::Down;
	}

	return HorizontalDot >= 0.0f ? EUxtPalmCameraDirection::Right : EUxtPalmCameraDirection::Left;
}

bool FUxtPalmDirectionUtility::IsDirectionAllowed(int32 AllowedPalmDirs, EUxtPalmCameraDirection CurrentDirectionBit)
{
	return (AllowedPalmDirs & static_cast<int32>(CurrentDirectionBit)) != 0;
}

bool FUxtPalmDirectionUtility::BuildTrackedHandFrame(
	const FXRMotionControllerData& MotionControllerData,
	const FTransform& TrackingToWorld,
	uint64 FrameNumber,
	float TimeSeconds,
	FUxtTrackedHandFrame& OutHandFrame)
{
	OutHandFrame.Reset(MotionControllerData.HandIndex);
	OutHandFrame.FrameNumber = FrameNumber;
	OutHandFrame.TimeSeconds = TimeSeconds;

	const bool bIsValidHand = MotionControllerData.bValid && MotionControllerData.DeviceVisualType == EXRVisualType::Hand &&
		MotionControllerData.HandKeyPositions.Num() == EHandKeypointCount &&
		MotionControllerData.HandKeyRotations.Num() == EHandKeypointCount;
	if (!bIsValidHand)
	{
		return false;
	}

	OutHandFrame.bIsTracked = MotionControllerData.TrackingStatus == ETrackingStatus::Tracked;
	OutHandFrame.EnsureArrayShape();

	for (int32 JointIndex = 0; JointIndex < EHandKeypointCount; ++JointIndex)
	{
		const FVector WorldLocation = TrackingToWorld.TransformPosition(MotionControllerData.HandKeyPositions[JointIndex]);
		const FQuat WorldRotation = TrackingToWorld.GetRotation() * MotionControllerData.HandKeyRotations[JointIndex];
		OutHandFrame.WorldSpaceJointTransforms[JointIndex] = FTransform(WorldRotation, WorldLocation);
		OutHandFrame.JointValidMask[JointIndex] = true;
	}

	FTransform WristTransform;
	if (!OutHandFrame.GetJointWorldTransform(EHandKeypoint::Wrist, WristTransform))
	{
		OutHandFrame.Reset(MotionControllerData.HandIndex);
		return false;
	}

	OutHandFrame.WristTransform = WristTransform;

	const FVector PalmPosition = GetJointWorldPosition(OutHandFrame, EHandKeypoint::Palm);
	const FVector LittleBasePosition = GetJointWorldPosition(OutHandFrame, EHandKeypoint::LittleMetacarpal);
	const FVector IndexBasePosition = GetJointWorldPosition(OutHandFrame, EHandKeypoint::IndexMetacarpal);

	OutHandFrame.PalmBasis.Origin = WristTransform.GetLocation();
	OutHandFrame.PalmBasis.Across = (LittleBasePosition - PalmPosition).GetSafeNormal();
	OutHandFrame.PalmBasis.Up = (IndexBasePosition - PalmPosition).GetSafeNormal();
	OutHandFrame.PalmBasis.Normal = FVector::CrossProduct(OutHandFrame.PalmBasis.Across, OutHandFrame.PalmBasis.Up).GetSafeNormal();

	OutHandFrame.PalmWidth = FVector::Distance(IndexBasePosition, LittleBasePosition);
	return true;
}

void FUxtHandPoseMirrorUtility::MirrorHandData(const FUxtStaticPoseHandData& SourceHandData, FUxtStaticPoseHandData& OutMirroredHandData)
{
	OutMirroredHandData = SourceHandData;
	OutMirroredHandData.EnsureArrayShape();
	OutMirroredHandData.ReferenceWristTransform.SetLocation(
		OutMirroredHandData.ReferenceWristTransform.GetLocation().MirrorByVector(FVector::RightVector));
	OutMirroredHandData.ReferenceWristTransform.SetRotation(
		MirrorRotationByYZPlane(OutMirroredHandData.ReferenceWristTransform.GetRotation()));

	for (FUxtPoseJointPositionSample& PositionSample : OutMirroredHandData.JointPositions)
	{
		PositionSample.LocalPosition = PositionSample.LocalPosition.MirrorByVector(FVector::RightVector);
	}

	for (FUxtPoseJointRotationSample& RotationSample : OutMirroredHandData.JointRotations)
	{
		RotationSample.LocalRotation = MirrorRotationByYZPlane(RotationSample.LocalRotation);
	}

	OutMirroredHandData.bIsValid = SourceHandData.bIsValid;
}
