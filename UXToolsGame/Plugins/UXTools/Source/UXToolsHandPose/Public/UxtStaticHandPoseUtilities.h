#pragma once

#include "CoreMinimal.h"
#include "HeadMountedDisplayTypes.h"

#include "UxtStaticPoseDefinition.h"
#include "UxtStaticPoseEvaluator.h"

class UXTOOLSHANDPOSE_API FUxtPalmDirectionUtility
{
public:
	static EUxtPalmCameraDirection GetPalmDirectionBit(const FUxtTrackedHandFrame& HandFrame, const FTransform& CameraTransform);
	static bool IsDirectionAllowed(int32 AllowedPalmDirs, EUxtPalmCameraDirection CurrentDirectionBit);
	static bool BuildTrackedHandFrame(
		const struct FXRMotionControllerData& MotionControllerData,
		const FTransform& TrackingToWorld,
		uint64 FrameNumber,
		float TimeSeconds,
		FUxtTrackedHandFrame& OutHandFrame);
};

class UXTOOLSHANDPOSE_API FUxtHandPoseMirrorUtility
{
public:
	static void MirrorHandData(const FUxtStaticPoseHandData& SourceHandData, FUxtStaticPoseHandData& OutMirroredHandData);
};
