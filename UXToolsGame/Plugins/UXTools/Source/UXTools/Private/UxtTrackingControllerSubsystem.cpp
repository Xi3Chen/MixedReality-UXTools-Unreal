// Fill out your copyright notice in the Description page of Project Settings.


#include "UxtTrackingControllerSubsystem.h"
#include "IXRTrackingSystem.h"
#include "HeadMountedDisplayTypes.h"
#include "Camera/CameraComponent.h"
#include "IHandTracker.h"
#include "IXRCamera.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

UCameraComponent* UUxtTrackingControllerSubsystem::GetPlayerFollowCameraComponent() const
{
	if(PlayerFollowCameraComponent && !IsValid(PlayerFollowCameraComponent))
	{
		PlayerFollowCameraComponent = nullptr;
	}
	return PlayerFollowCameraComponent;   
}

void UUxtTrackingControllerSubsystem::SetPlayerFollowCameraComponent(UCameraComponent* NewPlayerFollowCameraComponent)
{
	PlayerFollowCameraComponent = NewPlayerFollowCameraComponent;
	// 触发相机更新事件
	OnPlayerFollowCameraUpdated.Broadcast(NewPlayerFollowCameraComponent);
}

void UUxtTrackingControllerSubsystem::GetXRMotionControllerData(UObject* WorldContextObject,EControllerHand Hand,bool bUsinWorldSpace,FXRMotionControllerData& MotionControllerData)
{
	MotionControllerData.bValid = false;
	MotionControllerData.DeviceName = NAME_None;
	MotionControllerData.ApplicationInstanceID = FApp::GetInstanceId();
	MotionControllerData.DeviceVisualType = EXRVisualType::Controller;
	MotionControllerData.TrackingStatus = ETrackingStatus::NotTracked;
	MotionControllerData.HandIndex = Hand;
	if(!GEngine->XRSystem.IsValid())
	{
		return;
	}
	GEngine->XRSystem->GetMotionControllerData(WorldContextObject, Hand, MotionControllerData);
	if(!MotionControllerData.bValid)
	{
		return;
	}
	FTransform CachedTrackingToWorld = ComputeTrackingToWorldTransform(WorldContextObject);
	FName HandTrackerName("OpenXRHandTracking");
	TArray<IHandTracker*> HandTrackers = IModularFeatures::Get().GetModularFeatureImplementations<IHandTracker>(IHandTracker::GetModularFeatureName());
	IHandTracker* HandTracker = nullptr;
	for (auto Itr : HandTrackers)
	{
		if (Itr->GetHandTrackerDeviceTypeName() == HandTrackerName)
		{
			HandTracker = Itr;
			break;
		}
	}

	FName MotionControllerName("OpenXR");
	TArray<IMotionController*> MotionControllers = IModularFeatures::Get().GetModularFeatureImplementations<IMotionController>(IMotionController::GetModularFeatureName());
	IMotionController* MotionController = nullptr;
	IMotionController* handMotionController = nullptr;
	for (auto Itr : MotionControllers)
	{
		if (Itr->GetMotionControllerDeviceTypeName() == MotionControllerName)
		{
			MotionController = Itr;
			break;
		}
	}
	for (auto Itr : MotionControllers)
	{
		if (Itr->GetMotionControllerDeviceTypeName() == HandTrackerName)
		{
			handMotionController = Itr;
			break;
		}
	}
	if (handMotionController)
	{
		const float WorldToMeters = GEngine->XRSystem->GetWorldToMetersScale();

		bool bSuccess = false;
		FVector Position = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;
		FTransform trackingToWorld = FTransform(FRotator(0.0f),FVector(0.0f));
		if(bUsinWorldSpace)
		{
			trackingToWorld = CachedTrackingToWorld;
		}
		FName AimSource = Hand == EControllerHand::Left ? FName("Left") : FName("Right");
		// UE_LOG(LogHMD, Log, TEXT("GetMotionControllerData GetControllerOrientationAndPosition 22."));
		bSuccess = handMotionController->GetControllerOrientationAndPosition(0, AimSource, Rotation, Position, WorldToMeters);
		if (bSuccess)
		{
			MotionControllerData.AimPosition = trackingToWorld.TransformPosition(Position);
			MotionControllerData.AimRotation = trackingToWorld.TransformRotation(FQuat(Rotation));
		}
		// UE_LOG(LogHMD, Log, TEXT("GetMotionControllerData GetControllerOrientationAndPosition 22 AimRotation.X = %f.%f"), MotionControllerData.AimPosition.X, MotionControllerData.AimPosition.Y);
		MotionControllerData.bValid |= bSuccess;

		FName GripSource = Hand == EControllerHand::Left ? FName("Left") : FName("Right");
		bSuccess = handMotionController->GetControllerOrientationAndPosition(0, GripSource, Rotation, Position, WorldToMeters);
		if (bSuccess)
		{
			MotionControllerData.GripPosition = trackingToWorld.TransformPosition(Position);
			MotionControllerData.GripRotation = trackingToWorld.TransformRotation(FQuat(Rotation));
		}
		MotionControllerData.bValid |= bSuccess;

		MotionControllerData.TrackingStatus = handMotionController->GetControllerTrackingStatus(0, GripSource);
	} else if (MotionController)
	{
		const float WorldToMeters = GEngine->XRSystem->GetWorldToMetersScale();

		bool bSuccess = false;
		FVector Position = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;
		FTransform trackingToWorld = FTransform(FRotator(0.0f),FVector(0.0f));
		if(bUsinWorldSpace)
		{
			trackingToWorld = CachedTrackingToWorld;
		}
		FName AimSource = Hand == EControllerHand::Left ? FName("LeftAim") : FName("RightAim");
		bSuccess = MotionController->GetControllerOrientationAndPosition(0, AimSource, Rotation, Position, WorldToMeters);
		if (bSuccess)
		{
			MotionControllerData.AimPosition = trackingToWorld.TransformPosition(Position);
			MotionControllerData.AimRotation = trackingToWorld.TransformRotation(FQuat(Rotation));
		}
		// UE_LOG(LogHMD, Log, TEXT("GetMotionControllerData GetControllerOrientationAndPosition AimPosition.X = %f."), MotionControllerData.AimPosition.X);
		MotionControllerData.bValid |= bSuccess;

		FName GripSource = Hand == EControllerHand::Left ? FName("LeftGrip") : FName("RightGrip");
		bSuccess = MotionController->GetControllerOrientationAndPosition(0, GripSource, Rotation, Position, WorldToMeters);
		if (bSuccess)
		{
			MotionControllerData.GripPosition = trackingToWorld.TransformPosition(Position);
			MotionControllerData.GripRotation = trackingToWorld.TransformRotation(FQuat(Rotation));
		}
		MotionControllerData.bValid |= bSuccess;

		MotionControllerData.TrackingStatus = MotionController->GetControllerTrackingStatus(0, GripSource);
	}
	
	if (HandTracker && HandTracker->IsHandTrackingStateValid())
	{
		if (bUsinWorldSpace)
		{
			FTransform oldTrackingToWorld = GEngine->XRSystem->GetTrackingToWorldTransform();
			GEngine->XRSystem->UpdateTrackingToWorldTransform(CachedTrackingToWorld);
			bool bSuccess = false;
			MotionControllerData.DeviceVisualType = EXRVisualType::Hand;

			MotionControllerData.bValid = HandTracker->GetAllKeypointStates(Hand, MotionControllerData.HandKeyPositions, MotionControllerData.HandKeyRotations, MotionControllerData.HandKeyRadii);
			check(!MotionControllerData.bValid || (MotionControllerData.HandKeyPositions.Num() == EHandKeypointCount && MotionControllerData.HandKeyRotations.Num() == EHandKeypointCount && MotionControllerData.HandKeyRadii.Num() == EHandKeypointCount));
			GEngine->XRSystem->UpdateTrackingToWorldTransform(oldTrackingToWorld);

		}
		else
		{
			FTransform oldTrackingToWorld = GEngine->XRSystem->GetTrackingToWorldTransform();
			GEngine->XRSystem->UpdateTrackingToWorldTransform(FTransform(FRotator::ZeroRotator, FVector::ZeroVector));

			bool bSuccess = false;
			MotionControllerData.DeviceVisualType = EXRVisualType::Hand;

			MotionControllerData.bValid = HandTracker->GetAllKeypointStates(Hand, MotionControllerData.HandKeyPositions, MotionControllerData.HandKeyRotations, MotionControllerData.HandKeyRadii);
			check(!MotionControllerData.bValid || (MotionControllerData.HandKeyPositions.Num() == EHandKeypointCount && MotionControllerData.HandKeyRotations.Num() == EHandKeypointCount && MotionControllerData.HandKeyRadii.Num() == EHandKeypointCount));
			GEngine->XRSystem->UpdateTrackingToWorldTransform(oldTrackingToWorld);
		}
	}

	//TODO: this is reportedly a wmr specific convenience function for rapid prototyping.  Not sure it is useful for openxr.
	MotionControllerData.bIsGrasped = false;
}

FTransform UUxtTrackingControllerSubsystem::ComputeTrackingToWorldTransform(UObject* WorldContextObject) const
{
	//Copy from XRTrackingSystemBase.cpp file. func :ComputeTrackingToWorldTransform
	FTransform TrackingToWorld = FTransform::Identity;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject,EGetWorldErrorMode::ReturnNull);
	if (World)
	{
		// Get the primary player for this world context
		const ULocalPlayer* XRPlayer = GEngine->GetFirstGamePlayer(World);

		if (XRPlayer)
		{
			const APlayerController* PlayerController = XRPlayer->GetPlayerController(World);
			if (PlayerController)
			{
				if(!GEngine->XRSystem.IsValid())
				{
					return TrackingToWorld;
				}
				const AActor* PlayerViewTarget = PlayerController->GetViewTarget();
				// follows ULocalPlayer::GetProjectionData()'s logic - where there's two different  
				// modes we use for determining the HMD's view: implicit vs. explicit...
				// IMPLICIT: the player has a camera component which represents the HMD, meaning 
				//           that component's parent is the tracking origin
				// EXPLICIT: there is no object representing the HMD, so we assume the ViewTaget
				//           is the tracking origin and offset the HMD from that
				auto XRCamera = GEngine->XRSystem->GetXRCamera();
				const bool bUsesImplicitHMDPositioning = XRCamera.IsValid() ? XRCamera->GetUseImplicitHMDPosition() : 
					PlayerViewTarget && PlayerViewTarget->HasActiveCameraComponent();

				if (bUsesImplicitHMDPositioning)
				{
					TArray<UCameraComponent*> CameraComponents;
					PlayerViewTarget->GetComponents<UCameraComponent>(CameraComponents);

					UCameraComponent* PlayerCamera = nullptr;
					for (UCameraComponent* Cam : CameraComponents)
					{
						// emulates AActor::CalcCamera(); the PlayerCameraManager uses ViewTarget->CalcCamera() for 
						// the HMD view's basis (regardless of whether bLockToHmd is set), CalcCamera() just finds 
						// the first active cam component and chooses that
						if (Cam->IsActive())
						{
							PlayerCamera = Cam;
							break;
						}
					}
					// @TODO: PlayerCamera can be null if we're, say, using debug camera functionality. We should think about supporting these scenarios.
					if (PlayerCamera)
					{
						USceneComponent* ViewParent = PlayerCamera->GetAttachParent();
						if (ViewParent)
						{
							TrackingToWorld = ViewParent->GetComponentTransform();

							if (PlayerCamera->IsUsingAbsoluteLocation())
							{
								TrackingToWorld.SetLocation(FVector::ZeroVector);
							}

							if (PlayerCamera->IsUsingAbsoluteRotation())
							{
								TrackingToWorld.SetRotation(FQuat::Identity);
							}

							if (PlayerCamera->IsUsingAbsoluteScale())
							{
								TrackingToWorld.SetScale3D(FVector::OneVector);
							}
						}
						// else, if the camera is the root component (not attached to an origin point)
						// then it is directly relative to the world - the tracking origin is the world's origin (i.e. the identity)
					}
				}
				// if we don't have a camera component, then the HMD is relative to the player's  
				// ViewPoint (see FDefaultXRCamera::CalculateStereoCameraOffset), which means that the
				// ViewPoint is treated as the tracking origin
				else
				{
					FVector  ViewPos;
					// NOTE: the player's view point will have the HMD's rotation folded into it (see 
					//       APlayerController::UpdateRotation => FDefaultXRCamera::ApplyHMDRotation)
					//       so the ViewPoint's rotation doesn't wholly represent the tracking origin's orientation
					FRotator ViewRot;
					PlayerController->GetPlayerViewPoint(ViewPos, ViewRot);

					// in FDefaultXRCamera::ApplyHMDRotation(), we clear the player's ViewRotation, and 
					// replace it with: the frame's yaw delta + hmd orientation; this has two implications...
					//     1) The HMD is initially relative to the world (not the player's rotation)
					//     2) The tracking origin can be directly rotated with player input, and isn't static
					if (GEngine->XRSystem.IsValid())
					{
						FVector HMDPos;
						FQuat   HMDRot;
						if (GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, HMDRot, HMDPos))
						{
							// @TODO: This is assuming users are using the PlayerController's default implementation for
							//        UpdateRotation(). If they're not calling ApplyHMDRotation() then this is wrong,
							//        and ViewRot should be left unmodified.
							ViewRot = FRotator(ViewRot.Quaternion() * HMDRot.Inverse());
						}
					}

					TrackingToWorld = FTransform(ViewRot, ViewPos);
				}
			}
		}

		// don't need to incorporate the world scale here, as its expected that the XR system
		// backend reports tracking space poses with that already incorporated
		// 
		// @TODO: we should probably move world scale up to this level, instead of having every 
		//        system handle it individually
		//TrackingToWorld.SetScale3D(TrackingToWorld->GetScale3D() * GetWorldToMetersScale(WorldContext));
	}
		// backend reports tracking space poses with that already incorporated
		// 
		// @TODO: we should probably move world scale up to this level, instead of having every 
		//        system handle it individually

	return TrackingToWorld;
}
