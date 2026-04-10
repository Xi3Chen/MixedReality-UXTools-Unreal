#include "UxtStaticPoseEvaluator.h"

#include "UxtStaticPoseDefinition.h"

namespace
{
	float SafeNormalizeScore(float Error, float Tolerance)
	{
		if (Tolerance <= KINDA_SMALL_NUMBER)
		{
			return Error <= KINDA_SMALL_NUMBER ? 1.0f : 0.0f;
		}

		return FMath::Clamp(1.0f - (Error / Tolerance), 0.0f, 1.0f);
	}
}

void FUxtTrackedHandFrame::Reset(EControllerHand InHand)
{
	bIsTracked = false;
	Hand = InHand;
	WorldSpaceJointTransforms.Reset();
	JointValidMask.Reset();
	WristTransform = FTransform::Identity;
	PalmBasis = FUxtPalmBasis();
	PalmWidth = 0.0f;
	FrameNumber = 0;
	TimeSeconds = 0.0f;
	EnsureArrayShape();
}

void FUxtTrackedHandFrame::EnsureArrayShape()
{
	if (WorldSpaceJointTransforms.Num() != EHandKeypointCount)
	{
		WorldSpaceJointTransforms.SetNum(EHandKeypointCount);
	}

	if (JointValidMask.Num() != EHandKeypointCount)
	{
		JointValidMask.Init(false, EHandKeypointCount);
	}
}

bool FUxtTrackedHandFrame::HasValidJoint(EHandKeypoint Joint) const
{
	const int32 Index = static_cast<int32>(Joint);
	return JointValidMask.IsValidIndex(Index) && JointValidMask[Index] && WorldSpaceJointTransforms.IsValidIndex(Index);
}

bool FUxtTrackedHandFrame::GetJointWorldTransform(EHandKeypoint Joint, FTransform& OutTransform) const
{
	const int32 Index = static_cast<int32>(Joint);
	if (!HasValidJoint(Joint))
	{
		return false;
	}

	OutTransform = WorldSpaceJointTransforms[Index];
	return true;
}

bool FUxtTrackedHandFrame::GetJointLocalTransform(EHandKeypoint Joint, FTransform& OutTransform) const
{
	FTransform JointWorldTransform;
	if (!GetJointWorldTransform(Joint, JointWorldTransform))
	{
		return false;
	}

	OutTransform = JointWorldTransform.GetRelativeTransform(WristTransform);
	return true;
}

bool FUxtTrackedHandFrame::GetJointLocalRotation(EHandKeypoint Joint, FQuat& OutRotation) const
{
	FTransform LocalTransform;
	if (!GetJointLocalTransform(Joint, LocalTransform))
	{
		return false;
	}

	OutRotation = LocalTransform.GetRotation();
	return true;
}

bool FUxtTrackedHandFrame::GetJointLocalPosition(EHandKeypoint Joint, FVector& OutPosition) const
{
	FTransform LocalTransform;
	if (!GetJointLocalTransform(Joint, LocalTransform))
	{
		return false;
	}

	OutPosition = LocalTransform.GetLocation();
	return true;
}

FUxtStaticPoseEvaluator::FUxtStaticPoseEvaluator(const UUxtStaticPoseDefinition& InDefinition)
	: Definition(InDefinition)
{
}

bool FUxtStaticPoseEvaluator::TryGetReferenceHandData(
	const FUxtTrackedHandFrame& CurrentHandData,
	const FUxtStaticPoseHandData*& OutReferenceHandData) const
{
	if (!CurrentHandData.bIsTracked)
	{
		OutReferenceHandData = nullptr;
		return false;
	}

	return Definition.GetPoseDataForHand(CurrentHandData.Hand, OutReferenceHandData);
}

bool FUxtStaticPoseEvaluator::CheckRequiredJoint(
	const FUxtTrackedHandFrame& CurrentHandData,
	const FUxtStaticPoseHandData& ReferenceHandData,
	EHandKeypoint Joint,
	FUxtStaticPoseEvaluationResult& OutResult) const
{
	if (!CurrentHandData.HasValidJoint(Joint) || !ReferenceHandData.HasValidJoint(Joint))
	{
		OutResult = FUxtStaticPoseEvaluationResult();
		OutResult.FailureReason = EUxtStaticPoseFailureReason::MissingJoint;
		OutResult.DebugJoint = Joint;
		return false;
	}

	return true;
}

FUxtStaticPoseRotationEvaluator::FUxtStaticPoseRotationEvaluator(const UUxtStaticPoseRotationDefinition& InDefinition)
	: FUxtStaticPoseEvaluator(InDefinition)
	, RotationDefinition(InDefinition)
{
}

bool FUxtStaticPoseRotationEvaluator::Evaluate(
	const FUxtTrackedHandFrame& CurrentHandData,
	FUxtStaticPoseEvaluationResult& OutResult) const
{
	OutResult = FUxtStaticPoseEvaluationResult();

	const FUxtStaticPoseHandData* ReferenceHandData = nullptr;
	if (!TryGetReferenceHandData(CurrentHandData, ReferenceHandData) || !ReferenceHandData)
	{
		OutResult.FailureReason = CurrentHandData.bIsTracked ? EUxtStaticPoseFailureReason::MissingPoseData :
			EUxtStaticPoseFailureReason::HandNotTracked;
		return false;
	}

	const TArray<TEnumAsByte<EHandKeypoint>>& Joints = RotationDefinition.Joints;
	if (Joints.IsEmpty())
	{
		OutResult.FailureReason = EUxtStaticPoseFailureReason::MissingPoseData;
		return false;
	}

	float TotalAngleError = 0.0f;
	for (EHandKeypoint Joint : Joints)
	{
		if (!CheckRequiredJoint(CurrentHandData, *ReferenceHandData, Joint, OutResult))
		{
			return false;
		}

		FQuat CurrentRotation;
		FQuat ReferenceRotation;
		CurrentHandData.GetJointLocalRotation(Joint, CurrentRotation);
		ReferenceHandData->GetLocalRotation(Joint, ReferenceRotation);

		const float AngleDegrees = FMath::RadiansToDegrees(CurrentRotation.AngularDistance(ReferenceRotation));
		TotalAngleError += AngleDegrees;
		OutResult.PrimaryDebugValue = FMath::Max(OutResult.PrimaryDebugValue, AngleDegrees);
	}

	const float AverageAngleError = TotalAngleError / static_cast<float>(Joints.Num());
	OutResult.PrimaryDebugValue = AverageAngleError;
	OutResult.Score = SafeNormalizeScore(AverageAngleError, RotationDefinition.MaxAverageAngleDegrees);
	OutResult.bPassed = AverageAngleError <= RotationDefinition.MaxAverageAngleDegrees;
	OutResult.FailureReason = OutResult.bPassed ? EUxtStaticPoseFailureReason::None : EUxtStaticPoseFailureReason::BelowThreshold;
	return OutResult.bPassed;
}

FUxtStaticPoseBoundsEvaluator::FUxtStaticPoseBoundsEvaluator(const UUxtStaticPoseBoundsDefinition& InDefinition)
	: FUxtStaticPoseEvaluator(InDefinition)
	, BoundsDefinition(InDefinition)
{
}

bool FUxtStaticPoseBoundsEvaluator::Evaluate(
	const FUxtTrackedHandFrame& CurrentHandData,
	FUxtStaticPoseEvaluationResult& OutResult) const
{
	OutResult = FUxtStaticPoseEvaluationResult();

	const FUxtStaticPoseHandData* ReferenceHandData = nullptr;
	if (!TryGetReferenceHandData(CurrentHandData, ReferenceHandData) || !ReferenceHandData)
	{
		OutResult.FailureReason = CurrentHandData.bIsTracked ? EUxtStaticPoseFailureReason::MissingPoseData :
			EUxtStaticPoseFailureReason::HandNotTracked;
		return false;
	}

	int32 EnabledRuleCount = 0;
	int32 PassingRuleCount = 0;
	float TotalRuleScore = 0.0f;

	for (const FUxtStaticPoseJointBounds& Rule : BoundsDefinition.JointBounds)
	{
		if (!Rule.bEnabled)
		{
			continue;
		}

		++EnabledRuleCount;
		if (!CheckRequiredJoint(CurrentHandData, *ReferenceHandData, Rule.Joint, OutResult))
		{
			return false;
		}

		FVector CurrentPosition;
		FVector ReferencePosition;
		CurrentHandData.GetJointLocalPosition(Rule.Joint, CurrentPosition);
		ReferenceHandData->GetLocalPosition(Rule.Joint, ReferencePosition);

		float Scale = 1.0f;
		if (BoundsDefinition.bNormalizeByPalmWidth && CurrentHandData.PalmWidth > KINDA_SMALL_NUMBER)
		{
			Scale = CurrentHandData.PalmWidth;
		}

		const FVector Delta = (CurrentPosition - ReferencePosition) / Scale;
		bool bRulePassed = false;
		float RuleError = 0.0f;

		if (Rule.Shape == EUxtStaticPoseBoundsShape::Sphere)
		{
			RuleError = Delta.Size();
			bRulePassed = RuleError <= Rule.SphereRadius;
			TotalRuleScore += SafeNormalizeScore(RuleError, Rule.SphereRadius);
		}
		else
		{
			const FVector NormalizedAbs = Delta.GetAbs();
			RuleError = FMath::Max3(NormalizedAbs.X, NormalizedAbs.Y, NormalizedAbs.Z);
			bRulePassed =
				NormalizedAbs.X <= Rule.BoxTolerance.X && NormalizedAbs.Y <= Rule.BoxTolerance.Y &&
				NormalizedAbs.Z <= Rule.BoxTolerance.Z;

			const float BoxToleranceMax = FMath::Max3(Rule.BoxTolerance.X, Rule.BoxTolerance.Y, Rule.BoxTolerance.Z);
			TotalRuleScore += SafeNormalizeScore(RuleError, BoxToleranceMax);
		}

		if (bRulePassed)
		{
			++PassingRuleCount;
		}

		OutResult.PrimaryDebugValue = FMath::Max(OutResult.PrimaryDebugValue, RuleError);
		if (!bRulePassed)
		{
			OutResult.DebugJoint = Rule.Joint;
		}
	}

	if (EnabledRuleCount == 0)
	{
		OutResult.FailureReason = EUxtStaticPoseFailureReason::MissingPoseData;
		return false;
	}

	const float PassingRatio = static_cast<float>(PassingRuleCount) / static_cast<float>(EnabledRuleCount);
	OutResult.Score = TotalRuleScore / static_cast<float>(EnabledRuleCount);
	OutResult.bPassed = PassingRatio >= BoundsDefinition.MinPassingRatio;
	OutResult.FailureReason = OutResult.bPassed ? EUxtStaticPoseFailureReason::None : EUxtStaticPoseFailureReason::BelowThreshold;
	OutResult.PrimaryDebugValue = PassingRatio;
	return OutResult.bPassed;
}

FUxtStaticPoseDistanceEvaluator::FUxtStaticPoseDistanceEvaluator(const UUxtStaticPoseDistanceDefinition& InDefinition)
	: FUxtStaticPoseEvaluator(InDefinition)
	, DistanceDefinition(InDefinition)
{
}

bool FUxtStaticPoseDistanceEvaluator::Evaluate(
	const FUxtTrackedHandFrame& CurrentHandData,
	FUxtStaticPoseEvaluationResult& OutResult) const
{
	OutResult = FUxtStaticPoseEvaluationResult();

	const FUxtStaticPoseHandData* ReferenceHandData = nullptr;
	if (!TryGetReferenceHandData(CurrentHandData, ReferenceHandData) || !ReferenceHandData)
	{
		OutResult.FailureReason = CurrentHandData.bIsTracked ? EUxtStaticPoseFailureReason::MissingPoseData :
			EUxtStaticPoseFailureReason::HandNotTracked;
		return false;
	}

	float TotalWeight = 0.0f;
	float PassingWeight = 0.0f;
	float WeightedScore = 0.0f;

	for (const FUxtStaticPoseDistanceRule& Rule : DistanceDefinition.DistanceRules)
	{
		if (!Rule.bEnabled)
		{
			continue;
		}

		if (!CheckRequiredJoint(CurrentHandData, *ReferenceHandData, Rule.JointA, OutResult) ||
			!CheckRequiredJoint(CurrentHandData, *ReferenceHandData, Rule.JointB, OutResult))
		{
			return false;
		}

		FVector PositionA;
		FVector PositionB;
		CurrentHandData.GetJointLocalPosition(Rule.JointA, PositionA);
		CurrentHandData.GetJointLocalPosition(Rule.JointB, PositionB);

		float CurrentDistance = FVector::Distance(PositionA, PositionB);
		if (Rule.bNormalizeByPalmWidth && CurrentHandData.PalmWidth > KINDA_SMALL_NUMBER)
		{
			CurrentDistance /= CurrentHandData.PalmWidth;
		}

		const float Error = FMath::Abs(CurrentDistance - Rule.TargetDistance);
		const float Weight = FMath::Max(Rule.Weight, 0.0f);
		TotalWeight += Weight;
		WeightedScore += SafeNormalizeScore(Error, Rule.DistanceTolerance) * Weight;

		if (Error <= Rule.DistanceTolerance)
		{
			PassingWeight += Weight;
		}

		OutResult.PrimaryDebugValue = FMath::Max(OutResult.PrimaryDebugValue, Error);
	}

	if (TotalWeight <= KINDA_SMALL_NUMBER)
	{
		OutResult.FailureReason = EUxtStaticPoseFailureReason::MissingPoseData;
		return false;
	}

	const float PassingRatio = PassingWeight / TotalWeight;
	OutResult.Score = WeightedScore / TotalWeight;
	OutResult.bPassed = PassingRatio >= DistanceDefinition.MinWeightedPassingRatio;
	OutResult.FailureReason = OutResult.bPassed ? EUxtStaticPoseFailureReason::None : EUxtStaticPoseFailureReason::BelowThreshold;
	OutResult.PrimaryDebugValue = PassingRatio;
	return OutResult.bPassed;
}
