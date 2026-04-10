// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "HeadMountedDisplayTypes.h"

#include "HandTracking/IUxtHandTracker.h"

class AXRSimulationActor;
struct FXRSimulationState;

namespace UxtHandTrackerInputActions
{
	static const FName LeftSelect = TEXT("UxtLeftSelect");
	static const FName LeftGrab = TEXT("UxtLeftGrab");
	static const FName RightSelect = TEXT("UxtRightSelect");
	static const FName RightGrab = TEXT("UxtRightGrab");
} // namespace UxtHandTrackerInputActions

/** Default hand tracker implementation.
 *
 * This implementation works for all XR systems. It uses the XRTrackingSystem engine API.
 * Hand and controller data is based on the FXRMotionControllerData.
 *
 * Motion controller data is cached at the beginning of each frame.
 * Input events for known XR systems are used to keep track of Select and Grip actions.
 */
class FUxtDefaultHandTracker : public IUxtHandTracker
{
public:
	enum class HandJointID : uint8
	{
		Wrist,
		Palm,
		ThumbMetacarpal,
		ThumbProximal,
		ThumbDistal,
		ThumbTip,
		IndexMetacarpal,
		IndexProximal,//6
		IndexMiddle,
		IndexDistal,
		IndexTip,
		MiddleMetacarpal,
		MiddleProximal,//10
		MiddleMiddle,
		MiddleDistal,
		MiddleTip,
		RingMetacarpal,
		RingProximal,
		RingMiddle,
		RingDistal,
		RingTip,
		PinkyMetacarpal,
		PinkyProximal,
		PinkyMiddle,
		PinkyDistal,
		PinkyTip
	};
	static void RegisterInputMappings();
	static void UnregisterInputMappings();

	FXRMotionControllerData& GetControllerData(EControllerHand Hand);
	const FXRMotionControllerData& GetControllerData(EControllerHand Hand) const;
    int32 ConvertMRTKJointToXvXRJoint(EHandKeypoint joint) const;
	//
	// IUxtHandTracker interface

	virtual ETrackingStatus GetTrackingStatus(EControllerHand Hand) const override;
	virtual bool IsHandController(EControllerHand Hand) const override;
	virtual bool GetJointState(
		EControllerHand Hand, EHandKeypoint Joint, FQuat& OutOrientation, FVector& OutPosition, float& OutRadius) const override;
	virtual bool GetPointerPose(EControllerHand Hand, FQuat& OutOrientation, FVector& OutPosition) const override;
	virtual bool GetGripPose(EControllerHand Hand, FQuat& OutOrientation, FVector& OutPosition) const override;
	virtual bool GetIsGrabbing(EControllerHand Hand, bool& OutIsGrabbing) const override;
	virtual bool GetIsSelectPressed(EControllerHand Hand, bool& OutIsSelectPressed) const override;

private:
	FXRMotionControllerData ControllerData_Left;
	FXRMotionControllerData ControllerData_Right;
	bool bIsGrabbing_Left = false;
	bool bIsSelectPressed_Left = false;
	bool bIsGrabbing_Right = false;
	bool bIsSelectPressed_Right = false;
    bool isPinching = false;
	friend class UUxtDefaultHandTrackerSubsystem;
	static bool TransformConverterToFollowCamera(FQuat& OutOrientation, FVector& OutPosition);
};
