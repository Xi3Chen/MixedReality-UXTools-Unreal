#include "UxtStaticHandPoseDebugFunctionLibrary.h"

#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "HeadMountedDisplayTypes.h"
#include "UxtStaticHandPoseSubsystem.h"
#include "UxtStaticPoseDefinition.h"

namespace
{
	UUxtStaticHandPoseSubsystem* GetHandPoseSubsystem(UObject* WorldContextObject)
	{
		if (!WorldContextObject || !GEngine)
		{
			return nullptr;
		}

		return GEngine->GetEngineSubsystem<UUxtStaticHandPoseSubsystem>();
	}
}

bool UUxtStaticHandPoseDebugFunctionLibrary::GetTrackedHandFrame(
	UObject* WorldContextObject,
	EControllerHand Hand,
	FUxtTrackedHandFrame& OutHandFrame)
{
	if (UUxtStaticHandPoseSubsystem* Subsystem = GetHandPoseSubsystem(WorldContextObject))
	{
		return Subsystem->GetTrackedHandFrame(Hand, OutHandFrame);
	}

	return false;
}

bool UUxtStaticHandPoseDebugFunctionLibrary::EvaluatePoseDefinition(
	UObject* WorldContextObject,
	UUxtStaticPoseDefinition* PoseDefinition,
	EControllerHand Hand,
	FUxtStaticPoseEvaluationResult& OutResult)
{
	if (UUxtStaticHandPoseSubsystem* Subsystem = GetHandPoseSubsystem(WorldContextObject))
	{
		return Subsystem->EvaluatePoseDefinition(PoseDefinition, Hand, OutResult);
	}

	return false;
}

bool UUxtStaticHandPoseDebugFunctionLibrary::CapturePoseDefinition(
	UObject* WorldContextObject,
	UUxtStaticPoseDefinition* PoseDefinition,
	EControllerHand Hand)
{
	if (UUxtStaticHandPoseSubsystem* Subsystem = GetHandPoseSubsystem(WorldContextObject))
	{
		return Subsystem->CapturePoseDefinition(PoseDefinition, Hand);
	}

	return false;
}

void UUxtStaticHandPoseDebugFunctionLibrary::DrawDebugHand(
	UObject* WorldContextObject,
	const TArray<FVector>& HandKeyPositions,
	const TArray<FQuat>& HandKeyRotations,
	FLinearColor LineColor)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	const UEnum* KeypointEnum = StaticEnum<EHandKeypoint>();
	const int32 KeypointCount = KeypointEnum->GetMaxEnumValue();

	if (!World || HandKeyPositions.Num() != KeypointCount ||
		HandKeyRotations.Num() != KeypointCount)
	{
		return;
	}

	const FColor Color = LineColor.ToFColor(true);

	for (int32 i = 0; i < KeypointCount; ++i)
	{
		DrawDebugSphere(World, HandKeyPositions[i], 1.0f, 12, Color);
	}

	auto DrawBone = [&](EHandKeypoint JointA, EHandKeypoint JointB) {
		DrawDebugLine(World, HandKeyPositions[(int32)JointA], HandKeyPositions[(int32)JointB], Color);
	};

	DrawBone(EHandKeypoint::Wrist, EHandKeypoint::ThumbMetacarpal);
	DrawBone(EHandKeypoint::ThumbMetacarpal, EHandKeypoint::ThumbProximal);
	DrawBone(EHandKeypoint::ThumbProximal, EHandKeypoint::ThumbDistal);
	DrawBone(EHandKeypoint::ThumbDistal, EHandKeypoint::ThumbTip);

	DrawBone(EHandKeypoint::Wrist, EHandKeypoint::IndexMetacarpal);
	DrawBone(EHandKeypoint::IndexMetacarpal, EHandKeypoint::IndexProximal);
	DrawBone(EHandKeypoint::IndexProximal, EHandKeypoint::IndexIntermediate);
	DrawBone(EHandKeypoint::IndexIntermediate, EHandKeypoint::IndexDistal);
	DrawBone(EHandKeypoint::IndexDistal, EHandKeypoint::IndexTip);

	DrawBone(EHandKeypoint::Wrist, EHandKeypoint::MiddleMetacarpal);
	DrawBone(EHandKeypoint::MiddleMetacarpal, EHandKeypoint::MiddleProximal);
	DrawBone(EHandKeypoint::MiddleProximal, EHandKeypoint::MiddleIntermediate);
	DrawBone(EHandKeypoint::MiddleIntermediate, EHandKeypoint::MiddleDistal);
	DrawBone(EHandKeypoint::MiddleDistal, EHandKeypoint::MiddleTip);

	DrawBone(EHandKeypoint::Wrist, EHandKeypoint::RingMetacarpal);
	DrawBone(EHandKeypoint::RingMetacarpal, EHandKeypoint::RingProximal);
	DrawBone(EHandKeypoint::RingProximal, EHandKeypoint::RingIntermediate);
	DrawBone(EHandKeypoint::RingIntermediate, EHandKeypoint::RingDistal);
	DrawBone(EHandKeypoint::RingDistal, EHandKeypoint::RingTip);

	DrawBone(EHandKeypoint::Wrist, EHandKeypoint::LittleMetacarpal);
	DrawBone(EHandKeypoint::LittleMetacarpal, EHandKeypoint::LittleProximal);
	DrawBone(EHandKeypoint::LittleProximal, EHandKeypoint::LittleIntermediate);
	DrawBone(EHandKeypoint::LittleIntermediate, EHandKeypoint::LittleDistal);
	DrawBone(EHandKeypoint::LittleDistal, EHandKeypoint::LittleTip);
}
