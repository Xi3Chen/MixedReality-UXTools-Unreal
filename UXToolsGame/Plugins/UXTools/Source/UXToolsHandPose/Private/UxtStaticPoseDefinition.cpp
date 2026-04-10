#include "UxtStaticPoseDefinition.h"

#include "UxtStaticHandPoseUtilities.h"
#include "UxtStaticPoseEvaluator.h"

namespace
{
	int32 GetJointIndex(EHandKeypoint Joint)
	{
		return static_cast<int32>(Joint);
	}

	int32 GetExpectedJointCount()
	{
		return EHandKeypointCount;
	}

	template <typename TValue>
	void EnsureIndexedArray(TArray<TValue>& Array)
	{
		if (Array.Num() != GetExpectedJointCount())
		{
			Array.SetNum(GetExpectedJointCount());
		}

		for (int32 Index = 0; Index < Array.Num(); ++Index)
		{
			Array[Index].Joint = static_cast<EHandKeypoint>(Index);
		}
	}
}

void FUxtStaticPoseHandData::Reset()
{
	JointRotations.Reset();
	JointPositions.Reset();
	ReferenceWristTransform = FTransform::Identity;
	ReferencePalmWidth = 0.0f;
	bIsValid = false;
	EnsureArrayShape();
}

void FUxtStaticPoseHandData::EnsureArrayShape()
{
	EnsureIndexedArray(JointRotations);
	EnsureIndexedArray(JointPositions);
}

bool FUxtStaticPoseHandData::HasValidJoint(EHandKeypoint Joint) const
{
	const int32 Index = GetJointIndex(Joint);
	return JointRotations.IsValidIndex(Index) && JointPositions.IsValidIndex(Index) && JointRotations[Index].bIsValid &&
		JointPositions[Index].bIsValid;
}

bool FUxtStaticPoseHandData::GetLocalRotation(EHandKeypoint Joint, FQuat& OutRotation) const
{
	const int32 Index = GetJointIndex(Joint);
	if (!JointRotations.IsValidIndex(Index) || !JointRotations[Index].bIsValid)
	{
		return false;
	}

	OutRotation = JointRotations[Index].LocalRotation;
	return true;
}

bool FUxtStaticPoseHandData::GetLocalPosition(EHandKeypoint Joint, FVector& OutPosition) const
{
	const int32 Index = GetJointIndex(Joint);
	if (!JointPositions.IsValidIndex(Index) || !JointPositions[Index].bIsValid)
	{
		return false;
	}

	OutPosition = JointPositions[Index].LocalPosition;
	return true;
}

void FUxtStaticPoseHandData::SetLocalRotation(EHandKeypoint Joint, const FQuat& LocalRotation, bool bInIsValid)
{
	EnsureArrayShape();
	const int32 Index = GetJointIndex(Joint);
	JointRotations[Index].LocalRotation = LocalRotation;
	JointRotations[Index].bIsValid = bInIsValid;
}

void FUxtStaticPoseHandData::SetLocalPosition(EHandKeypoint Joint, const FVector& LocalPosition, bool bInIsValid)
{
	EnsureArrayShape();
	const int32 Index = GetJointIndex(Joint);
	JointPositions[Index].LocalPosition = LocalPosition;
	JointPositions[Index].bIsValid = bInIsValid;
}

void FUxtStaticPoseHandData::MarkJointValid(EHandKeypoint Joint, bool bInIsValid)
{
	EnsureArrayShape();
	const int32 Index = GetJointIndex(Joint);
	JointRotations[Index].bIsValid = bInIsValid;
	JointPositions[Index].bIsValid = bInIsValid;
}

void FUxtStaticPoseHandData::RebuildValidity()
{
	bIsValid = false;
	for (int32 Index = 0; Index < JointRotations.Num() && Index < JointPositions.Num(); ++Index)
	{
		if (JointRotations[Index].bIsValid && JointPositions[Index].bIsValid)
		{
			bIsValid = true;
			return;
		}
	}
}

void FUxtStaticPoseHandData::CaptureFromTrackedHand(const FUxtTrackedHandFrame& HandFrame)
{
	Reset();
	ReferenceWristTransform = HandFrame.WristTransform;
	ReferencePalmWidth = HandFrame.PalmWidth;

	for (int32 JointIndex = 0; JointIndex < GetExpectedJointCount(); ++JointIndex)
	{
		const EHandKeypoint Joint = static_cast<EHandKeypoint>(JointIndex);
		FTransform LocalTransform;
		if (HandFrame.GetJointLocalTransform(Joint, LocalTransform))
		{
			SetLocalRotation(Joint, LocalTransform.GetRotation());
			SetLocalPosition(Joint, LocalTransform.GetLocation());
		}
		else
		{
			MarkJointValid(Joint, false);
		}
	}

	RebuildValidity();
}

UUxtStaticPoseDefinition::UUxtStaticPoseDefinition()
{
	LeftHandData.EnsureArrayShape();
	RightHandData.EnsureArrayShape();
}

bool UUxtStaticPoseDefinition::IsPoseDataValid() const
{
	return LeftHandData.bIsValid || RightHandData.bIsValid;
}

void UUxtStaticPoseDefinition::EnsureMirroredHandData()
{
	if (!bAutoMirrorMissingHandData)
	{
		return;
	}

	if (LeftHandData.bIsValid && !RightHandData.bIsValid)
	{
		FUxtHandPoseMirrorUtility::MirrorHandData(LeftHandData, RightHandData);
	}
	else if (RightHandData.bIsValid && !LeftHandData.bIsValid)
	{
		FUxtHandPoseMirrorUtility::MirrorHandData(RightHandData, LeftHandData);
	}
}

bool UUxtStaticPoseDefinition::GetPoseDataForHand(EControllerHand Hand, const FUxtStaticPoseHandData*& OutHandData) const
{
	if (Hand == EControllerHand::Left && LeftHandData.bIsValid)
	{
		OutHandData = &LeftHandData;
		return true;
	}

	if (Hand == EControllerHand::Right && RightHandData.bIsValid)
	{
		OutHandData = &RightHandData;
		return true;
	}

	OutHandData = nullptr;
	return false;
}

FUxtStaticPoseHandData* UUxtStaticPoseDefinition::GetMutablePoseDataForHand(EControllerHand Hand)
{
	if (Hand == EControllerHand::Left)
	{
		return &LeftHandData;
	}

	if (Hand == EControllerHand::Right)
	{
		return &RightHandData;
	}

	return nullptr;
}

bool UUxtStaticPoseDefinition::CapturePoseForHand(EControllerHand Hand, const FUxtTrackedHandFrame& HandFrame)
{
	FUxtStaticPoseHandData* HandData = GetMutablePoseDataForHand(Hand);
	if (!HandData || !HandFrame.bIsTracked)
	{
		return false;
	}

	HandData->CaptureFromTrackedHand(HandFrame);
	EnsureMirroredHandData();
	return true;
}

bool UUxtStaticPoseDefinition::EvaluateHand(const FUxtTrackedHandFrame& HandFrame, FUxtStaticPoseEvaluationResult& OutResult) const
{
	const TSharedPtr<FUxtStaticPoseEvaluator> Evaluator = CreateEvaluator();
	if (!Evaluator.IsValid())
	{
		OutResult = FUxtStaticPoseEvaluationResult();
		OutResult.FailureReason = EUxtStaticPoseFailureReason::MissingPoseData;
		return false;
	}

	return Evaluator->Evaluate(HandFrame, OutResult);
}

TSharedPtr<FUxtStaticPoseEvaluator> UUxtStaticPoseRotationDefinition::CreateEvaluator() const
{
	return MakeShared<FUxtStaticPoseRotationEvaluator>(*this);
}

TSharedPtr<FUxtStaticPoseEvaluator> UUxtStaticPoseBoundsDefinition::CreateEvaluator() const
{
	return MakeShared<FUxtStaticPoseBoundsEvaluator>(*this);
}

TSharedPtr<FUxtStaticPoseEvaluator> UUxtStaticPoseDistanceDefinition::CreateEvaluator() const
{
	return MakeShared<FUxtStaticPoseDistanceEvaluator>(*this);
}
