#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputCoreTypes.h"

#include "UxtStaticPoseDefinition.h"
#include "UxtStaticPoseEvaluator.h"

#include "UxtStaticHandPoseBindings.generated.h"

class UUxtStaticPoseDefinition;

UENUM(BlueprintType)
enum class EUxtStaticHandPoseBindingState : uint8
{
	Idle,
	CandidateDown,
	Pressed,
	CandidateUp,
};

UENUM(BlueprintType)
enum class EUxtHandPoseKeySlot : uint8
{
	None = 0,
	Slot1 = 1,
	Slot2 = 2,
	Slot3 = 3,
	Slot4 = 4,
	Slot5 = 5,
	Slot6 = 6,
	Slot7 = 7,
	Slot8 = 8,
	Slot9 = 9,
	Slot10 = 10,
};

USTRUCT(BlueprintType)
struct UXTOOLSHANDPOSE_API FUxtStaticHandPoseBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose")
	TObjectPtr<UUxtStaticPoseDefinition> PoseDefinitionAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose", meta = (Bitmask, BitmaskEnum = "EUxtPalmCameraDirection"))
	int32 AllowedPalmDirs = static_cast<int32>(EUxtPalmCameraDirection::Up) |
		static_cast<int32>(EUxtPalmCameraDirection::Down) |
		static_cast<int32>(EUxtPalmCameraDirection::Left) |
		static_cast<int32>(EUxtPalmCameraDirection::Right) |
		static_cast<int32>(EUxtPalmCameraDirection::Forward) |
		static_cast<int32>(EUxtPalmCameraDirection::Backward);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose")
	bool bAllowEitherHand = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose", meta = (ClampMin = "0.0"))
	float DebounceDownTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose", meta = (ClampMin = "0.0"))
	float DebounceUpTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose", meta = (ClampMin = "0.0"))
	float MinHoldTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose")
	FText DebugName;
};

USTRUCT(BlueprintType)
struct UXTOOLSHANDPOSE_API FUxtStaticHandPoseBindingDebugInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	EUxtHandPoseKeySlot Slot = EUxtHandPoseKeySlot::None;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	FKey Key;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	FText BindingName;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	TEnumAsByte<EControllerHand> TriggeringHand = EControllerHand::AnyHand;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	float Score = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	bool bPassed = false;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	EUxtStaticHandPoseBindingState State = EUxtStaticHandPoseBindingState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	FUxtStaticPoseEvaluationResult Evaluation;
};

UCLASS(BlueprintType)
class UXTOOLSHANDPOSE_API UUxtStaticHandPoseBindingsAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	const FUxtStaticHandPoseBinding* GetBinding(EUxtHandPoseKeySlot Slot) const;
	bool IsValidSlot(EUxtHandPoseKeySlot Slot) const;
	int32 GetBindingCount() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose", meta = (TitleProperty = "DebugName"))
	TMap<EUxtHandPoseKeySlot, FUxtStaticHandPoseBinding> Bindings;
};
