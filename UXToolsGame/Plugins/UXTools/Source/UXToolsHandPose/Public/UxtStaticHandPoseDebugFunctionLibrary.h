#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "UxtStaticPoseEvaluator.h"

#include "UxtStaticHandPoseDebugFunctionLibrary.generated.h"

class UUxtStaticPoseDefinition;

UCLASS()
class UXTOOLSHANDPOSE_API UUxtStaticHandPoseDebugFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Gets the latest tracked joint positions and rotations for the specified hand.
	 *
	 * @param WorldContextObject Context object used to resolve the hand pose subsystem.
	 * @param Hand Hand to query.
	 * @param OutHandFrame Receives the tracked hand frame when available.
	 * @return True when a tracked frame was retrieved successfully.
	 */
	UFUNCTION(BlueprintPure, Category = "UXTools|Hand Pose", meta = (WorldContext = "WorldContextObject"))
	static bool GetTrackedHandFrame(UObject* WorldContextObject, EControllerHand Hand, FUxtTrackedHandFrame& OutHandFrame);

	/**
	 * Evaluates a static pose definition against the current tracked state of the specified hand.
	 *
	 * @param WorldContextObject Context object used to resolve the hand pose subsystem.
	 * @param PoseDefinition Pose asset to evaluate.
	 * @param Hand Hand to evaluate.
	 * @param OutResult Receives the evaluation result for the current hand pose.
	 * @return True when the evaluation completed successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "UXTools|Hand Pose", meta = (WorldContext = "WorldContextObject"))
	static bool EvaluatePoseDefinition(
		UObject* WorldContextObject,
		UUxtStaticPoseDefinition* PoseDefinition,
		EControllerHand Hand,
		FUxtStaticPoseEvaluationResult& OutResult);

	/**
	 * Captures the current tracked hand pose and writes it into the provided pose definition asset.
	 *
	 * @param WorldContextObject Context object used to resolve the hand pose subsystem.
	 * @param PoseDefinition Pose asset to populate with the current tracked hand pose.
	 * @param Hand Hand to capture.
	 * @return True when the pose was captured successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "UXTools|Hand Pose", meta = (WorldContext = "WorldContextObject"))
	static bool CapturePoseDefinition(UObject* WorldContextObject, UUxtStaticPoseDefinition* PoseDefinition, EControllerHand Hand);

	/**
	 * Draws a debug visualization for a hand skeleton using tracked joint positions.
	 *
	 * @param WorldContextObject Context object used to resolve the world for debug drawing.
	 * @param HandKeyPositions Joint positions indexed by EHandKeypoint.
	 * @param HandKeyRotations Joint rotations indexed by EHandKeypoint. The array length is validated to match the positions array.
	 * @param LineColor Linear color used for joints and bone lines.
	 */
	UFUNCTION(BlueprintCallable, Category = "UXTools|Hand Pose", meta = (WorldContext = "WorldContextObject"))
	static void DrawDebugHand(
		UObject* WorldContextObject,
		const TArray<FVector>& HandKeyPositions,
		const TArray<FQuat>& HandKeyRotations,
		FLinearColor LineColor);
};
