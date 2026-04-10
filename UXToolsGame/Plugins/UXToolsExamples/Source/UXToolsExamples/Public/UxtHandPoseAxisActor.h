#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputCoreTypes.h"
#include "UxtStaticHandPoseBindings.h"

#include "UxtHandPoseAxisActor.generated.h"

class UArrowComponent;
class USceneComponent;
class UUxtStaticHandPoseSubsystem;

UENUM(BlueprintType)
enum class EUxtHandPoseAxisDirection : uint8
{
	None,
	Up,
	Down,
	Left,
	Right,
	Forward,
	Backward,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUxtHandPoseAxisDirectionDelegate, EUxtHandPoseAxisDirection, Direction);

UCLASS(Blueprintable)
class UXTOOLSEXAMPLES_API AUxtHandPoseAxisActor : public AActor
{
	GENERATED_BODY()

public:
	AUxtHandPoseAxisActor(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintAssignable, Category = "UXToolsExamples|Hand Pose")
	FUxtHandPoseAxisDirectionDelegate OnAxisActivated;

	UPROPERTY(BlueprintAssignable, Category = "UXToolsExamples|Hand Pose")
	FUxtHandPoseAxisDirectionDelegate OnAxisDeactivated;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "UXToolsExamples|Hand Pose")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "UXToolsExamples|Hand Pose")
	TObjectPtr<UArrowComponent> UpArrow;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "UXToolsExamples|Hand Pose")
	TObjectPtr<UArrowComponent> DownArrow;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "UXToolsExamples|Hand Pose")
	TObjectPtr<UArrowComponent> LeftArrow;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "UXToolsExamples|Hand Pose")
	TObjectPtr<UArrowComponent> RightArrow;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "UXToolsExamples|Hand Pose")
	TObjectPtr<UArrowComponent> ForwardArrow;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "UXToolsExamples|Hand Pose")
	TObjectPtr<UArrowComponent> BackwardArrow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UXToolsExamples|Hand Pose", meta = (ExposeOnSpawn = true))
	EUxtHandPoseKeySlot Slot = EUxtHandPoseKeySlot::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UXToolsExamples|Hand Pose", meta = (ClampMin = "0.0", ExposeOnSpawn = true))
	float MovementThresholdCm = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UXToolsExamples|Hand Pose", meta = (ClampMin = "0.0", ExposeOnSpawn = true))
	float ArrowLength = 36.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UXToolsExamples|Hand Pose", meta = (ExposeOnSpawn = true))
	FLinearColor InactiveColor = FLinearColor(0.35f, 0.35f, 0.35f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UXToolsExamples|Hand Pose", meta = (ExposeOnSpawn = true))
	FLinearColor ActiveColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);

private:
	UFUNCTION()
	void HandleKeyStateChanged(EUxtHandPoseKeySlot InSlot, bool bIsPressed, bool bIsLeftHand);

	bool TryGetTrackingSpacePalmLocation(EControllerHand Hand, FVector& OutPalmLocation) const;
	bool TryGetTrackingToWorldTransform(FTransform& OutTrackingToWorld) const;
	void UpdateActorTransformFromTrackingSpace();
	void SetActorActiveState(bool bInActive);
	void SetAllArrowsHidden(bool bShouldHide);
	void UpdateArrowLength();
	void SetAllArrowColors(const FLinearColor& Color);
	void UpdateActiveDirection(EUxtHandPoseAxisDirection NewDirection, bool bForceDeactivateAll);
	void ApplyArrowColors(EUxtHandPoseAxisDirection Direction);
	void BroadcastDeactivateForAllAxes();
	TArray<UArrowComponent*> GetArrowComponents() const;
	UArrowComponent* GetArrowForDirection(EUxtHandPoseAxisDirection Direction) const;
	EUxtHandPoseAxisDirection GetDirectionFromDelta(const FVector& TrackingSpaceDelta) const;

	TWeakObjectPtr<UUxtStaticHandPoseSubsystem> HandPoseSubsystem;
	bool bTrackingActive = false;
	bool bTrackedHandIsLeft = true;
	FVector StartPalmTrackingLocation = FVector::ZeroVector;
	EUxtHandPoseAxisDirection ActiveDirection = EUxtHandPoseAxisDirection::None;
};
