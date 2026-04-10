#pragma once

#include "CoreMinimal.h"
#include "HeadMountedDisplayTypes.h"

#include "UxtStaticPoseEvaluator.generated.h"

class UUxtStaticPoseBoundsDefinition;
class UUxtStaticPoseDefinition;
class UUxtStaticPoseDistanceDefinition;
class UUxtStaticPoseRotationDefinition;

UENUM(BlueprintType)
enum class EUxtStaticPoseFailureReason : uint8
{
	None,
	HandNotTracked,
	MissingPoseData,
	MissingJoint,
	PalmDirRejected,
	BelowThreshold,
};

USTRUCT(BlueprintType)
struct UXTOOLSHANDPOSE_API FUxtPalmBasis
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	FVector Origin = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	FVector Normal = FVector::ForwardVector;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	FVector Across = FVector::RightVector;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	FVector Up = FVector::UpVector;
};

USTRUCT(BlueprintType)
struct UXTOOLSHANDPOSE_API FUxtTrackedHandFrame
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	bool bIsTracked = false;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	TEnumAsByte<EControllerHand> Hand = EControllerHand::Left;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	TArray<FTransform> WorldSpaceJointTransforms;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	TArray<bool> JointValidMask;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	FTransform WristTransform = FTransform::Identity;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	FUxtPalmBasis PalmBasis;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	float PalmWidth = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	int32 FrameNumber = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	float TimeSeconds = 0.0f;

	void Reset(EControllerHand InHand);
	void EnsureArrayShape();
	bool HasValidJoint(EHandKeypoint Joint) const;
	bool GetJointWorldTransform(EHandKeypoint Joint, FTransform& OutTransform) const;
	bool GetJointLocalTransform(EHandKeypoint Joint, FTransform& OutTransform) const;
	bool GetJointLocalRotation(EHandKeypoint Joint, FQuat& OutRotation) const;
	bool GetJointLocalPosition(EHandKeypoint Joint, FVector& OutPosition) const;
};

USTRUCT(BlueprintType)
struct UXTOOLSHANDPOSE_API FUxtStaticPoseEvaluationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	float Score = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	bool bPassed = false;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	EUxtStaticPoseFailureReason FailureReason = EUxtStaticPoseFailureReason::None;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	float PrimaryDebugValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Pose")
	TEnumAsByte<EHandKeypoint> DebugJoint = EHandKeypoint::Palm;
};

struct UXTOOLSHANDPOSE_API FUxtStaticPoseEvaluator
{
	explicit FUxtStaticPoseEvaluator(const UUxtStaticPoseDefinition& InDefinition);
	virtual ~FUxtStaticPoseEvaluator() = default;

	virtual bool Evaluate(const FUxtTrackedHandFrame& CurrentHandData, FUxtStaticPoseEvaluationResult& OutResult) const = 0;

protected:
	bool TryGetReferenceHandData(const FUxtTrackedHandFrame& CurrentHandData, const struct FUxtStaticPoseHandData*& OutReferenceHandData) const;
	bool CheckRequiredJoint(
		const FUxtTrackedHandFrame& CurrentHandData,
		const struct FUxtStaticPoseHandData& ReferenceHandData,
		EHandKeypoint Joint,
		FUxtStaticPoseEvaluationResult& OutResult) const;

	const UUxtStaticPoseDefinition& Definition;
};

struct UXTOOLSHANDPOSE_API FUxtStaticPoseRotationEvaluator : public FUxtStaticPoseEvaluator
{
	explicit FUxtStaticPoseRotationEvaluator(const UUxtStaticPoseRotationDefinition& InDefinition);

	virtual bool Evaluate(const FUxtTrackedHandFrame& CurrentHandData, FUxtStaticPoseEvaluationResult& OutResult) const override;

private:
	const UUxtStaticPoseRotationDefinition& RotationDefinition;
};

struct UXTOOLSHANDPOSE_API FUxtStaticPoseBoundsEvaluator : public FUxtStaticPoseEvaluator
{
	explicit FUxtStaticPoseBoundsEvaluator(const UUxtStaticPoseBoundsDefinition& InDefinition);

	virtual bool Evaluate(const FUxtTrackedHandFrame& CurrentHandData, FUxtStaticPoseEvaluationResult& OutResult) const override;

private:
	const UUxtStaticPoseBoundsDefinition& BoundsDefinition;
};

struct UXTOOLSHANDPOSE_API FUxtStaticPoseDistanceEvaluator : public FUxtStaticPoseEvaluator
{
	explicit FUxtStaticPoseDistanceEvaluator(const UUxtStaticPoseDistanceDefinition& InDefinition);

	virtual bool Evaluate(const FUxtTrackedHandFrame& CurrentHandData, FUxtStaticPoseEvaluationResult& OutResult) const override;

private:
	const UUxtStaticPoseDistanceDefinition& DistanceDefinition;
};
