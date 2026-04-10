#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "UxtStaticHandPoseBindings.h"
#include "UxtStaticPoseEvaluator.h"

#include "UxtStaticHandPoseSubsystem.generated.h"

class UUxtStaticHandPoseBindingsAsset;
class UUxtStaticPoseDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FUxtStaticHandPoseKeyStateChangedDelegate,
	EUxtHandPoseKeySlot,
	Slot,
	bool,
	bIsPressed,
	bool,
	bIsLeftHand);

UCLASS()
class UXTOOLSHANDPOSE_API UUxtStaticHandPoseSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "UXTools|Hand Pose")
	bool GetTrackedHandFrame(EControllerHand Hand, FUxtTrackedHandFrame& OutHandFrame) const;

	UFUNCTION(BlueprintCallable, Category = "UXTools|Hand Pose")
	bool EvaluatePoseDefinition(UUxtStaticPoseDefinition* PoseDefinition, EControllerHand Hand, FUxtStaticPoseEvaluationResult& OutResult) const;

	UFUNCTION(BlueprintCallable, Category = "UXTools|Hand Pose")
	bool CapturePoseDefinition(UUxtStaticPoseDefinition* PoseDefinition, EControllerHand Hand);

	UFUNCTION(BlueprintCallable, Category = "UXTools|Hand Pose")
	void ReloadBindings();

	UFUNCTION(BlueprintPure, Category = "UXTools|Hand Pose")
	void GetLatestBindingDebugInfo(TArray<FUxtStaticHandPoseBindingDebugInfo>& OutDebugInfos) const;

	UPROPERTY(BlueprintAssignable, Category = "UXTools|Hand Pose")
	FUxtStaticHandPoseKeyStateChangedDelegate OnKeyStateChange;

	const TArray<FUxtStaticHandPoseBindingDebugInfo>& GetLatestBindingDebugInfoInternal() const;

private:
	struct FBindingRuntimeState;
	struct FBindingCandidate;
	struct FBindingEntry;

	void OnWorldPreActorTick(UWorld* World, ELevelTick TickType, float DeltaTime);
	void UpdateTrackedHands(UWorld* World);
	void EvaluateBindings(UWorld* World, float DeltaTime);
	FBindingCandidate EvaluateBindingForHand(EUxtHandPoseKeySlot Slot, EControllerHand Hand, const FTransform& CameraTransform) const;
	void UpdateBindingState(EUxtHandPoseKeySlot Slot, const FBindingCandidate& Candidate, float TimeSeconds);
	void ClearModuleKeyStates() const;
	bool GetCameraTransform(UWorld* World, FTransform& OutCameraTransform) const;
	const FUxtTrackedHandFrame& GetCachedHandFrame(EControllerHand Hand) const;
	FUxtTrackedHandFrame& GetMutableCachedHandFrame(EControllerHand Hand);

	FDelegateHandle TickDelegateHandle;
	TObjectPtr<UUxtStaticHandPoseBindingsAsset> ActiveBindingsAsset = nullptr;
	TArray<FBindingEntry> BindingEntries;
	mutable TArray<FUxtStaticHandPoseBindingDebugInfo> CachedBindingDebugInfos;
	FUxtTrackedHandFrame CachedLeftHandFrame;
	FUxtTrackedHandFrame CachedRightHandFrame;
};
