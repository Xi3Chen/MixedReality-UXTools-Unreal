#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HeadMountedDisplayTypes.h"

#include "UxtStaticPoseDefinition.generated.h"

struct FUxtStaticPoseEvaluator;
struct FUxtStaticPoseEvaluationResult;
struct FUxtTrackedHandFrame;

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EUxtPalmCameraDirection : uint8
{
	None = 0 UMETA(Hidden),
	Up = 1 << 0,
	Down = 1 << 1,
	Left = 1 << 2,
	Right = 1 << 3,
	Forward = 1 << 4,
	Backward = 1 << 5,
};
ENUM_CLASS_FLAGS(EUxtPalmCameraDirection);

UENUM(BlueprintType)
enum class EUxtStaticPoseBoundsShape : uint8
{
	Box,
	Sphere,
};

USTRUCT(BlueprintType)
struct UXTOOLSHANDPOSE_API FUxtPoseJointRotationSample
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	TEnumAsByte<EHandKeypoint> Joint = EHandKeypoint::Palm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	FQuat LocalRotation = FQuat::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	bool bIsValid = false;
};

USTRUCT(BlueprintType)
struct UXTOOLSHANDPOSE_API FUxtPoseJointPositionSample
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	TEnumAsByte<EHandKeypoint> Joint = EHandKeypoint::Palm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	FVector LocalPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	bool bIsValid = false;
};

USTRUCT(BlueprintType)
struct UXTOOLSHANDPOSE_API FUxtStaticPoseHandData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	TArray<FUxtPoseJointRotationSample> JointRotations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	TArray<FUxtPoseJointPositionSample> JointPositions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	FTransform ReferenceWristTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	float ReferencePalmWidth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	bool bIsValid = false;

	void Reset();
	void EnsureArrayShape();
	bool HasValidJoint(EHandKeypoint Joint) const;
	bool GetLocalRotation(EHandKeypoint Joint, FQuat& OutRotation) const;
	bool GetLocalPosition(EHandKeypoint Joint, FVector& OutPosition) const;
	void SetLocalRotation(EHandKeypoint Joint, const FQuat& LocalRotation, bool bInIsValid = true);
	void SetLocalPosition(EHandKeypoint Joint, const FVector& LocalPosition, bool bInIsValid = true);
	void MarkJointValid(EHandKeypoint Joint, bool bInIsValid);
	void RebuildValidity();
	void CaptureFromTrackedHand(const FUxtTrackedHandFrame& HandFrame);
};

USTRUCT(BlueprintType)
struct UXTOOLSHANDPOSE_API FUxtStaticPoseJointBounds
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	TEnumAsByte<EHandKeypoint> Joint = EHandKeypoint::Palm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	EUxtStaticPoseBoundsShape Shape = EUxtStaticPoseBoundsShape::Box;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	FVector BoxTolerance = FVector(2.0f, 2.0f, 2.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	float SphereRadius = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	bool bEnabled = true;
};

USTRUCT(BlueprintType)
struct UXTOOLSHANDPOSE_API FUxtStaticPoseDistanceRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	TEnumAsByte<EHandKeypoint> JointA = EHandKeypoint::ThumbTip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	TEnumAsByte<EHandKeypoint> JointB = EHandKeypoint::IndexTip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	float TargetDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	float DistanceTolerance = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	float Weight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	bool bNormalizeByPalmWidth = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Pose")
	bool bEnabled = true;
};

UCLASS(Abstract, BlueprintType)
class UXTOOLSHANDPOSE_API UUxtStaticPoseDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UUxtStaticPoseDefinition();

	virtual TSharedPtr<FUxtStaticPoseEvaluator> CreateEvaluator() const PURE_VIRTUAL(UUxtStaticPoseDefinition::CreateEvaluator, return nullptr;);
	virtual bool IsPoseDataValid() const;
	virtual void EnsureMirroredHandData();

	bool GetPoseDataForHand(EControllerHand Hand, const FUxtStaticPoseHandData*& OutHandData) const;
	FUxtStaticPoseHandData* GetMutablePoseDataForHand(EControllerHand Hand);
	bool CapturePoseForHand(EControllerHand Hand, const FUxtTrackedHandFrame& HandFrame);
	bool EvaluateHand(const FUxtTrackedHandFrame& HandFrame, FUxtStaticPoseEvaluationResult& OutResult) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose")
	FName PoseId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose")
	int32 Version = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose")
	bool bAutoMirrorMissingHandData = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose")
	FUxtStaticPoseHandData LeftHandData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose")
	FUxtStaticPoseHandData RightHandData;
};

UCLASS(BlueprintType)
class UXTOOLSHANDPOSE_API UUxtStaticPoseRotationDefinition : public UUxtStaticPoseDefinition
{
	GENERATED_BODY()

public:
	virtual TSharedPtr<FUxtStaticPoseEvaluator> CreateEvaluator() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose|Rotation")
	TArray<TEnumAsByte<EHandKeypoint>> Joints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose|Rotation", meta = (ClampMin = "0.1"))
	float MaxAverageAngleDegrees = 20.0f;
};

UCLASS(BlueprintType)
class UXTOOLSHANDPOSE_API UUxtStaticPoseBoundsDefinition : public UUxtStaticPoseDefinition
{
	GENERATED_BODY()

public:
	virtual TSharedPtr<FUxtStaticPoseEvaluator> CreateEvaluator() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose|Bounds")
	TArray<FUxtStaticPoseJointBounds> JointBounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose|Bounds")
	bool bNormalizeByPalmWidth = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose|Bounds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinPassingRatio = 1.0f;
};

UCLASS(BlueprintType)
class UXTOOLSHANDPOSE_API UUxtStaticPoseDistanceDefinition : public UUxtStaticPoseDefinition
{
	GENERATED_BODY()

public:
	virtual TSharedPtr<FUxtStaticPoseEvaluator> CreateEvaluator() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose|Distance")
	TArray<FUxtStaticPoseDistanceRule> DistanceRules;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Pose|Distance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinWeightedPassingRatio = 1.0f;
};
