// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/UxtEyePointerComponent.h"

#include "EyeTracking/IUxtEyeTracker.h"
#include "HandTracking/IUxtHandTracker.h"
#include "Input/UxtInputSubsystem.h"

UUxtEyePointerComponent::UUxtEyePointerComponent()
{
}

void UUxtEyePointerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	UActorComponent::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const IUxtEyeTracker& EyeTracker = bForceHmdTracker ? IUxtEyeTracker::GetHdmTracker() : IUxtEyeTracker::Get();
	if (EyeTracker.GetTrackingStatus() != EEyeTrackerStatus::Tracking)
	{
		SetEnabled(false);
		return;
	}

	FEyeTrackerGazeData GazeData;
	if (!EyeTracker.GetGazeData(GazeData))
	{
		SetEnabled(false);
		return;
	}

	const FVector Direction = GazeData.GazeDirection.GetSafeNormal();
	if (Direction.IsNearlyZero() && bForceHmdTracker != false)
	{
		return;
	}

	const FQuat NewOrientation = Direction.Rotation().Quaternion();
	const FVector NewOrigin = GazeData.GazeOrigin;

	OnPointerPoseUpdated(NewOrientation, NewOrigin);
	UpdateParameterCollection(GetHitPoint());

	// Press/release is still driven by hand select (pinch) to support gesture + gaze targeting.
	// For AnyHand we consider either hand selecting as pressed.
	bool bNewPressed = false;
	IUxtHandTracker& HandTracker = IUxtHandTracker::Get();
	if (Hand == EControllerHand::AnyHand)
	{
		const bool bLeftTracked = HandTracker.GetTrackingStatus(EControllerHand::Left) == ETrackingStatus::Tracked;
		const bool bRightTracked = HandTracker.GetTrackingStatus(EControllerHand::Right) == ETrackingStatus::Tracked;

		bool bLeftPressed = false;
		bool bRightPressed = false;
		if (bLeftTracked)
		{
			HandTracker.GetIsSelectPressed(EControllerHand::Left, bLeftPressed);
		}
		if (bRightTracked)
		{
			HandTracker.GetIsSelectPressed(EControllerHand::Right, bRightPressed);
		}
		bNewPressed = (bLeftPressed || bRightPressed);
	}
	else
	{
		if (HandTracker.GetTrackingStatus(Hand) == ETrackingStatus::Tracked)
		{
			HandTracker.GetIsSelectPressed(Hand, bNewPressed);
		}
	}
	SetPressed(bNewPressed);

	SetEnabled(true);
}

void UUxtEyePointerComponent::OnPointerPoseUpdated(const FQuat& NewOrientation, const FVector& NewOrigin)
{
	Super::OnPointerPoseUpdated(NewOrientation, NewOrigin);
}
