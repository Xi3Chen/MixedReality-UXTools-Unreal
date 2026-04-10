// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "UxtDefaultHandTracker.h"

#include "IXRTrackingSystem.h"
#include "UxtTrackingControllerSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Features/IModularFeatures.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogUxtDefaultHandTracker, Log, All);

namespace
{
	const TArray<FInputActionKeyMapping> ActionMappings(
		{// OpenXR MsftHandInteraction mapping
		 FInputActionKeyMapping(UxtHandTrackerInputActions::LeftSelect, FKey("OpenXRMsftHandInteraction_Left_Select_Axis")),
		 FInputActionKeyMapping(UxtHandTrackerInputActions::LeftGrab, FKey("OpenXRMsftHandInteraction_Left_Grip_Axis")),
		 FInputActionKeyMapping(UxtHandTrackerInputActions::RightSelect, FKey("OpenXRMsftHandInteraction_Right_Select_Axis")),
		 FInputActionKeyMapping(UxtHandTrackerInputActions::RightGrab, FKey("OpenXRMsftHandInteraction_Right_Grip_Axis")),

		 // MixedReality mapping
		 FInputActionKeyMapping(UxtHandTrackerInputActions::LeftSelect, FKey("MixedReality_Left_Trigger_Click")),
		 FInputActionKeyMapping(UxtHandTrackerInputActions::LeftGrab, FKey("MixedReality_Left_Grip_Click")),
		 FInputActionKeyMapping(UxtHandTrackerInputActions::RightSelect, FKey("MixedReality_Right_Trigger_Click")),
		 FInputActionKeyMapping(UxtHandTrackerInputActions::RightGrab, FKey("MixedReality_Right_Grip_Click")),

		 // Oculus Touch mappings
		 FInputActionKeyMapping(UxtHandTrackerInputActions::LeftSelect, FKey("OculusTouch_Left_Trigger_Click")),
		 FInputActionKeyMapping(UxtHandTrackerInputActions::LeftGrab, FKey("OculusTouch_Left_Grip_Click")),
		 FInputActionKeyMapping(UxtHandTrackerInputActions::RightSelect, FKey("OculusTouch_Right_Trigger_Click")),
		 FInputActionKeyMapping(UxtHandTrackerInputActions::RightGrab, FKey("OculusTouch_Right_Grip_Click")),

		 // XRInputSimulation mapping
		 FInputActionKeyMapping(UxtHandTrackerInputActions::LeftSelect, FKey("XRSimulation_Left_Select")),
		 FInputActionKeyMapping(UxtHandTrackerInputActions::LeftGrab, FKey("XRSimulation_Left_Grip")),
		 FInputActionKeyMapping(UxtHandTrackerInputActions::RightSelect, FKey("XRSimulation_Right_Select")),
		 FInputActionKeyMapping(UxtHandTrackerInputActions::RightGrab, FKey("XRSimulation_Right_Grip"))});

	bool IsValidHandData(const FXRMotionControllerData& MotionControllerData)
	{
		if (MotionControllerData.DeviceVisualType == EXRVisualType::Hand && MotionControllerData.bValid)
		{
			check(
				MotionControllerData.HandKeyPositions.Num() == EHandKeypointCount &&
				MotionControllerData.HandKeyRotations.Num() == EHandKeypointCount &&
				MotionControllerData.HandKeyRadii.Num() == EHandKeypointCount);
			return true;
		}
		return false;
	}
} // namespace

void FUxtDefaultHandTracker::RegisterInputMappings()
{
	UInputSettings* InputSettings = GetMutableDefault<UInputSettings>();
	if (!InputSettings)
	{
		UE_LOG(LogUxtDefaultHandTracker, Warning, TEXT("Could not find mutable input settings"));
		return;
	}

	for (const FInputActionKeyMapping& Mapping : ActionMappings)
	{
		if (Mapping.Key.IsValid())
		{
			InputSettings->AddActionMapping(Mapping, false);
		}
	}
	InputSettings->ForceRebuildKeymaps();
}

void FUxtDefaultHandTracker::UnregisterInputMappings()
{
	UInputSettings* InputSettings = GetMutableDefault<UInputSettings>();
	if (!InputSettings)
	{
		return;
	}

	for (const FInputActionKeyMapping& Mapping : ActionMappings)
	{
		InputSettings->RemoveActionMapping(Mapping, false);
	}
	InputSettings->ForceRebuildKeymaps();
}

FXRMotionControllerData& FUxtDefaultHandTracker::GetControllerData(EControllerHand Hand)
{
	return Hand == EControllerHand::Left ? ControllerData_Left : ControllerData_Right;
}

const FXRMotionControllerData& FUxtDefaultHandTracker::GetControllerData(EControllerHand Hand) const
{
	return Hand == EControllerHand::Left ? ControllerData_Left : ControllerData_Right;
}

ETrackingStatus FUxtDefaultHandTracker::GetTrackingStatus(EControllerHand Hand) const
{
	const FXRMotionControllerData& MotionControllerData = GetControllerData(Hand);
	return MotionControllerData.bValid ? MotionControllerData.TrackingStatus : ETrackingStatus::NotTracked;
}

bool FUxtDefaultHandTracker::IsHandController(EControllerHand Hand) const
{
	const FXRMotionControllerData& MotionControllerData = GetControllerData(Hand);
	return IsValidHandData(MotionControllerData);
}
  int32 FUxtDefaultHandTracker::ConvertMRTKJointToXvXRJoint(EHandKeypoint joint) const
{
	  switch (joint)
	  {
	  case EHandKeypoint::Palm: return (int32)HandJointID::Palm;//AttachmentPointFlags.Palm;
	  case EHandKeypoint::Wrist: return (int32)HandJointID::Wrist;//AttachmentPointFlags.Wrist;

	  case EHandKeypoint::ThumbProximal: return (int32)HandJointID::ThumbProximal;//AttachmentPointFlags.ThumbProximalJoint;
	  case EHandKeypoint::ThumbDistal: return (int32)HandJointID::ThumbDistal;//AttachmentPointFlags.ThumbDistalJoint;
	  case EHandKeypoint::ThumbTip: return (int32)HandJointID::ThumbTip;//AttachmentPointFlags.ThumbTip;
	  case EHandKeypoint::ThumbMetacarpal: return (int32)HandJointID::ThumbMetacarpal;

	  case EHandKeypoint::IndexMetacarpal: return (int32)HandJointID::IndexMetacarpal;//AttachmentPointFlags.IndexKnuckle;
	  case EHandKeypoint::IndexProximal: return (int32)HandJointID::IndexProximal;//AttachmentPointFlags.IndexKnuckle;
	  case EHandKeypoint::IndexIntermediate: return (int32)HandJointID::IndexMiddle;//.IndexMiddleJoint;
	  case EHandKeypoint::IndexDistal: return (int32)HandJointID::IndexDistal;//AttachmentPointFlags.IndexDistalJoint;
	  case EHandKeypoint::IndexTip: return (int32)HandJointID::IndexTip;//AttachmentPointFlags.IndexTip;

	  case EHandKeypoint::MiddleMetacarpal: return (int32)HandJointID::MiddleMetacarpal;
	  case EHandKeypoint::MiddleProximal: return (int32)HandJointID::MiddleProximal;//AttachmentPointFlags.MiddleKnuckle;
	  case EHandKeypoint::MiddleIntermediate: return (int32)HandJointID::MiddleMiddle;//AttachmentPointFlags.MiddleMiddleJoint;
	  case EHandKeypoint::MiddleDistal: return (int32)HandJointID::MiddleDistal;//AttachmentPointFlags.MiddleDistalJoint;
	  case EHandKeypoint::MiddleTip: return (int32)HandJointID::MiddleTip;//AttachmentPointFlags.MiddleTip;

	  case EHandKeypoint::RingMetacarpal: return (int32)HandJointID::RingMetacarpal;//AttachmentPointFlags.RingKnuckle;
	  case EHandKeypoint::RingProximal: return (int32)HandJointID::RingProximal;//AttachmentPointFlags.RingKnuckle;
	  case EHandKeypoint::RingIntermediate: return (int32)HandJointID::RingMiddle;//AttachmentPointFlags.RingMiddleJoint;
	  case EHandKeypoint::RingDistal: return (int32)HandJointID::RingDistal;//AttachmentPointFlags.RingDistalJoint;
	  case EHandKeypoint::RingTip: return (int32)HandJointID::RingTip;//AttachmentPointFlags.RingTip;

	  case EHandKeypoint::LittleProximal: return (int32)HandJointID::PinkyProximal;//AttachmentPointFlags.PinkyKnuckle;
	  case EHandKeypoint::LittleIntermediate: return (int32)HandJointID::PinkyMiddle;//AttachmentPointFlags.PinkyMiddleJoint;
	  case EHandKeypoint::LittleDistal: return (int32)HandJointID::PinkyDistal;//AttachmentPointFlags.PinkyDistalJoint;
	  case EHandKeypoint::LittleTip: return (int32)HandJointID::PinkyTip;//AttachmentPointFlags.PinkyTip;
	  case EHandKeypoint::LittleMetacarpal: return (int32)HandJointID::PinkyMetacarpal;

		  // Metacarpals are not included in AttachmentPointFlags
	  default: return (int32)HandJointID::Palm;//AttachmentPointFlags.Wrist;
	  }
}

bool FUxtDefaultHandTracker::GetJointState(
	EControllerHand Hand, EHandKeypoint Joint, FQuat& OutOrientation, FVector& OutPosition, float& OutRadius) const
{
	const FXRMotionControllerData& MotionControllerData = GetControllerData(Hand);
	if (IsValidHandData(MotionControllerData))
	{
		const int32 iJoint = (int32)Joint;
		OutOrientation = MotionControllerData.HandKeyRotations[iJoint];
		OutPosition = MotionControllerData.HandKeyPositions[iJoint];
		if(TransformConverterToFollowCamera(OutOrientation,OutPosition))
		{
			OutRadius = MotionControllerData.HandKeyRadii[iJoint];
			return true;
		}
	}
	return false;
}

bool FUxtDefaultHandTracker::GetPointerPose(EControllerHand Hand, FQuat& OutOrientation, FVector& OutPosition) const
{
	const FXRMotionControllerData& MotionControllerData = GetControllerData(Hand);
	if (MotionControllerData.bValid)
	{
		OutOrientation = MotionControllerData.AimRotation;
		OutPosition = MotionControllerData.AimPosition;
		return TransformConverterToFollowCamera(OutOrientation,OutPosition);
		// UE_LOG(LogUxtDefaultHandTracker, Error, TEXT("eddy FUxtDefaultHandTracker GetPointerPose OutOrientation x y = %f,%f,%f,%f"), OutOrientation.X, OutOrientation.Y, OutOrientation.Z, OutOrientation.W);
		return true;
	}
	return false;
}

bool FUxtDefaultHandTracker::GetGripPose(EControllerHand Hand, FQuat& OutOrientation, FVector& OutPosition) const
{
	const FXRMotionControllerData& MotionControllerData = GetControllerData(Hand);
	if (MotionControllerData.bValid)
	{
		OutOrientation = MotionControllerData.GripRotation;
		OutPosition = MotionControllerData.GripPosition;
		
		return TransformConverterToFollowCamera(OutOrientation,OutPosition);

	}
	return false;
}

bool FUxtDefaultHandTracker::GetIsGrabbing(EControllerHand Hand, bool& OutIsGrabbing) const
{
	FQuat IndexRotation, RingRotation;
	FVector IndexLocation, RingLocation;
	float IndexRadius, RingRadius, distance;
	bool res;
	switch (Hand)
	{
	case EControllerHand::Left:
		res = GetJointState(EControllerHand::Left, EHandKeypoint::IndexTip, IndexRotation, IndexLocation, IndexRadius);
		GetJointState(EControllerHand::Left, EHandKeypoint::ThumbTip, RingRotation, RingLocation, RingRadius);

		distance = FVector::Dist(RingLocation, IndexLocation);

		if (distance > 2.0f || !res)
		{
			OutIsGrabbing = false;

		}
		else if (distance < 2.0f)
		{
			OutIsGrabbing = true;
		}
	//	OutIsGrabbing = bIsGrabbing_Left;
		return true;
	case EControllerHand::Right:
		res = GetJointState(EControllerHand::Right, EHandKeypoint::IndexTip, IndexRotation, IndexLocation, IndexRadius);
		GetJointState(EControllerHand::Right, EHandKeypoint::ThumbTip, RingRotation, RingLocation, RingRadius);

		distance = FVector::Dist(RingLocation, IndexLocation);

		if (distance > 2.0f || !res)
		{
			OutIsGrabbing = false;

		}
		else if (distance < 2.0f)
		{
			OutIsGrabbing = true;
		}
		//OutIsGrabbing = bIsGrabbing_Right;
		return true;
	}
	return false;
}

bool FUxtDefaultHandTracker::GetIsSelectPressed(EControllerHand Hand, bool& OutIsSelectPressed) const
{
	FQuat IndexRotation, RingRotation;
	FVector IndexLocation, RingLocation;
	float IndexRadius, RingRadius, distance;
	bool res;
	switch (Hand)
	{
	case EControllerHand::Left:
	
		 res = GetJointState(EControllerHand::Left, EHandKeypoint::IndexTip, IndexRotation, IndexLocation, IndexRadius);
		GetJointState(EControllerHand::Left, EHandKeypoint::ThumbTip, RingRotation, RingLocation, RingRadius);
		
		 distance = FVector::Dist(RingLocation, IndexLocation);

		if (distance > 2.0f || !res)
		{
			OutIsSelectPressed = false;
		
		}
		else if (distance < 2.0f)
		{
			OutIsSelectPressed = true;
		
		}
	//	OutIsSelectPressed = bIsSelectPressed_Left;
		return true;
	case EControllerHand::Right:
		res = GetJointState(EControllerHand::Right, EHandKeypoint::IndexTip, IndexRotation, IndexLocation, IndexRadius);
		GetJointState(EControllerHand::Right, EHandKeypoint::ThumbTip, RingRotation, RingLocation, RingRadius);

		 distance = FVector::Dist(RingLocation, IndexLocation);
		//  UE_LOG(LogUxtDefaultHandTracker, Error, TEXT("eddy FUxtDefaultHandTracker GetIsSelectPressed distance = %f"), distance);
		if (distance > 3.0f || !res)
		{
			OutIsSelectPressed = false;
		}
		else if (distance < 3.0f)
		{
			OutIsSelectPressed = true;
		}
	//	OutIsSelectPressed = bIsSelectPressed_Right;
		return true;
	}
	return false;
}

bool FUxtDefaultHandTracker::TransformConverterToFollowCamera(FQuat& OutOrientation, FVector& OutPosition)
{
	if(OutOrientation.ContainsNaN() || OutPosition.ContainsNaN())
	{
		//ensureMsgf(0,TEXT("OutOrientation or OutPosition contain nan"));
		return false;
	}
	if(auto PlayerFollowCameraComponent = GEngine->GetEngineSubsystem<UUxtTrackingControllerSubsystem>()->GetPlayerFollowCameraComponent())
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(PlayerFollowCameraComponent, 0);
		if (PC != nullptr && PlayerFollowCameraComponent->IsActive() && PlayerFollowCameraComponent->GetOwner() ==
			PC->GetViewTarget() && GEngine->XRSystem.
			IsValid())
		{
			FTransform PointerTransform;

			FTransform NewXRTransform = PlayerFollowCameraComponent->GetAttachParent()->GetComponentTransform();
			FTransform XRToWorldTransform = GEngine->XRSystem->GetTrackingToWorldTransform();
			PointerTransform.SetLocation(XRToWorldTransform.InverseTransformPosition(OutPosition));
			PointerTransform.SetRotation(XRToWorldTransform.InverseTransformRotation(OutOrientation));
			PointerTransform = PointerTransform*PlayerFollowCameraComponent->GetAttachParent()->GetComponentTransform();

			OutOrientation = PointerTransform.GetRotation();
			OutPosition = PointerTransform.GetLocation();
			return true;
		}
		return false;
	}
	return false;
}
