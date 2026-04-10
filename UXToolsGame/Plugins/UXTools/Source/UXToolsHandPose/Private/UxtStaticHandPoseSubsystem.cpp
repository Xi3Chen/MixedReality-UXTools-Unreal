#include "UxtStaticHandPoseSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "IXRTrackingSystem.h"
#include "UxtStaticHandPoseSettings.h"
#include "UxtStaticHandPoseUtilities.h"
#include "UxtStaticPoseDefinition.h"
#include "UXToolsHandPose.h"

namespace
{
}

struct UUxtStaticHandPoseSubsystem::FBindingCandidate
{
	bool bValid = false;
	bool bPassed = false;
	EUxtHandPoseKeySlot Slot = EUxtHandPoseKeySlot::None;
	FKey Key;
	EControllerHand Hand = EControllerHand::AnyHand;
	float Score = 0.0f;
	int32 Priority = 0;
	FUxtStaticPoseEvaluationResult Evaluation;
};

struct UUxtStaticHandPoseSubsystem::FBindingRuntimeState
{
	EUxtStaticHandPoseBindingState State = EUxtStaticHandPoseBindingState::Idle;
	bool bPhysicallyPressed = false;
	EControllerHand TriggeringHand = EControllerHand::AnyHand;
	float StateChangeTime = 0.0f;
	float PressedStartTime = 0.0f;
};

struct UUxtStaticHandPoseSubsystem::FBindingEntry
{
	EUxtHandPoseKeySlot Slot = EUxtHandPoseKeySlot::None;
	FBindingRuntimeState RuntimeState;
	FUxtStaticHandPoseBindingDebugInfo DebugInfo;
};

void UUxtStaticHandPoseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CachedLeftHandFrame.Reset(EControllerHand::Left);
	CachedRightHandFrame.Reset(EControllerHand::Right);
	ReloadBindings();
	TickDelegateHandle = FWorldDelegates::OnWorldPreActorTick.AddUObject(this, &UUxtStaticHandPoseSubsystem::OnWorldPreActorTick);
}

void UUxtStaticHandPoseSubsystem::Deinitialize()
{
	if (TickDelegateHandle.IsValid())
	{
		FWorldDelegates::OnWorldPreActorTick.Remove(TickDelegateHandle);
		TickDelegateHandle.Reset();
	}

	ClearModuleKeyStates();
	BindingEntries.Reset();
	CachedBindingDebugInfos.Reset();
	ActiveBindingsAsset = nullptr;

	Super::Deinitialize();
}

bool UUxtStaticHandPoseSubsystem::GetTrackedHandFrame(EControllerHand Hand, FUxtTrackedHandFrame& OutHandFrame) const
{
	const FUxtTrackedHandFrame& CachedFrame = GetCachedHandFrame(Hand);
	OutHandFrame = CachedFrame;
	return CachedFrame.bIsTracked;
}

bool UUxtStaticHandPoseSubsystem::EvaluatePoseDefinition(
	UUxtStaticPoseDefinition* PoseDefinition,
	EControllerHand Hand,
	FUxtStaticPoseEvaluationResult& OutResult) const
{
	if (!PoseDefinition)
	{
		return false;
	}

	return PoseDefinition->EvaluateHand(GetCachedHandFrame(Hand), OutResult);
}

bool UUxtStaticHandPoseSubsystem::CapturePoseDefinition(UUxtStaticPoseDefinition* PoseDefinition, EControllerHand Hand)
{
	if (!PoseDefinition)
	{
		return false;
	}

	return PoseDefinition->CapturePoseForHand(Hand, GetCachedHandFrame(Hand));
}

void UUxtStaticHandPoseSubsystem::ReloadBindings()
{
	const UUxtStaticHandPoseSettings* Settings = GetDefault<UUxtStaticHandPoseSettings>();
	ActiveBindingsAsset = Settings ? Settings->BindingsAsset.LoadSynchronous() : nullptr;

	BindingEntries.Reset();
	CachedBindingDebugInfos.Reset();

	if (ActiveBindingsAsset)
	{
		for (const TPair<EUxtHandPoseKeySlot, FUxtStaticHandPoseBinding>& Pair : ActiveBindingsAsset->Bindings)
		{
			if (Pair.Key == EUxtHandPoseKeySlot::None)
			{
				continue;
			}

			FBindingEntry Entry;
			Entry.Slot = Pair.Key;
			Entry.DebugInfo.Slot = Pair.Key;
			Entry.DebugInfo.Key = UxtHandPoseKeys::GetKeyForSlot(Pair.Key);
			Entry.DebugInfo.BindingName = Pair.Value.DebugName;
			if (Pair.Value.PoseDefinitionAsset)
			{
				Pair.Value.PoseDefinitionAsset->EnsureMirroredHandData();
			}

			BindingEntries.Add(MoveTemp(Entry));
		}
	}

	ClearModuleKeyStates();
}

void UUxtStaticHandPoseSubsystem::GetLatestBindingDebugInfo(TArray<FUxtStaticHandPoseBindingDebugInfo>& OutDebugInfos) const
{
	OutDebugInfos.Reset(BindingEntries.Num());
	for (const FBindingEntry& Entry : BindingEntries)
	{
		OutDebugInfos.Add(Entry.DebugInfo);
	}
	CachedBindingDebugInfos = OutDebugInfos;
}

const TArray<FUxtStaticHandPoseBindingDebugInfo>& UUxtStaticHandPoseSubsystem::GetLatestBindingDebugInfoInternal() const
{
	CachedBindingDebugInfos.Reset(BindingEntries.Num());
	for (const FBindingEntry& Entry : BindingEntries)
	{
		CachedBindingDebugInfos.Add(Entry.DebugInfo);
	}
	return CachedBindingDebugInfos;
}

void UUxtStaticHandPoseSubsystem::OnWorldPreActorTick(UWorld* World, ELevelTick TickType, float DeltaTime)
{
	if (!World || World->IsPreviewWorld())
	{
		return;
	}

	UpdateTrackedHands(World);
	EvaluateBindings(World, DeltaTime);
}

void UUxtStaticHandPoseSubsystem::UpdateTrackedHands(UWorld* World)
{
	CachedLeftHandFrame.Reset(EControllerHand::Left);
	CachedRightHandFrame.Reset(EControllerHand::Right);

	if (!World || !GEngine)
	{
		return;
	}

	IXRTrackingSystem* XRSystem = GEngine->XRSystem.Get();
	if (!XRSystem)
	{
		return;
	}

	const FTransform CachedTrackingToWorld(FRotator::ZeroRotator, FVector::ZeroVector);
	const FTransform OldTrackingToWorld = XRSystem->GetTrackingToWorldTransform();
	XRSystem->UpdateTrackingToWorldTransform(CachedTrackingToWorld);

	FXRMotionControllerData LeftData;
	XRSystem->GetMotionControllerData(World, EControllerHand::Left, LeftData);
	FUxtPalmDirectionUtility::BuildTrackedHandFrame(
		LeftData, CachedTrackingToWorld, GFrameCounter, World->GetTimeSeconds(), CachedLeftHandFrame);

	FXRMotionControllerData RightData;
	XRSystem->GetMotionControllerData(World, EControllerHand::Right, RightData);
	FUxtPalmDirectionUtility::BuildTrackedHandFrame(
		RightData, CachedTrackingToWorld, GFrameCounter, World->GetTimeSeconds(), CachedRightHandFrame);

	XRSystem->UpdateTrackingToWorldTransform(OldTrackingToWorld);
}

void UUxtStaticHandPoseSubsystem::EvaluateBindings(UWorld* World, float DeltaTime)
{
	if (!ActiveBindingsAsset)
	{
		ClearModuleKeyStates();
		return;
	}

	FTransform CameraTransform;
	if (!GetCameraTransform(World, CameraTransform))
	{
		ClearModuleKeyStates();
		return;
	}

	for (FBindingEntry& Entry : BindingEntries)
	{
		const FUxtStaticHandPoseBinding* Binding = ActiveBindingsAsset->GetBinding(Entry.Slot);
		if (!Binding)
		{
			continue;
		}

		FBindingCandidate BestCandidate;
		BestCandidate.Slot = Entry.Slot;
		BestCandidate.Key = UxtHandPoseKeys::GetKeyForSlot(Entry.Slot);

		TArray<EControllerHand, TInlineAllocator<2>> HandsToEvaluate;
		HandsToEvaluate.Add(EControllerHand::Left);
		if (Binding->bAllowEitherHand)
		{
			HandsToEvaluate.Add(EControllerHand::Right);
		}

		for (EControllerHand Hand : HandsToEvaluate)
		{
			const FBindingCandidate Candidate = EvaluateBindingForHand(Entry.Slot, Hand, CameraTransform);
			if (!Candidate.bValid)
			{
				continue;
			}

			if (!BestCandidate.bValid || Candidate.Score > BestCandidate.Score ||
				(FMath::IsNearlyEqual(Candidate.Score, BestCandidate.Score) && Candidate.Priority > BestCandidate.Priority) ||
				(FMath::IsNearlyEqual(Candidate.Score, BestCandidate.Score) && Candidate.Priority == BestCandidate.Priority &&
					Entry.RuntimeState.TriggeringHand == Candidate.Hand))
			{
				BestCandidate = Candidate;
			}
		}

		UpdateBindingState(Entry.Slot, BestCandidate, World->GetTimeSeconds());
	}
}

UUxtStaticHandPoseSubsystem::FBindingCandidate UUxtStaticHandPoseSubsystem::EvaluateBindingForHand(
	EUxtHandPoseKeySlot Slot,
	EControllerHand Hand,
	const FTransform& CameraTransform) const
{
	FBindingCandidate Candidate;
	Candidate.Slot = Slot;
	Candidate.Key = UxtHandPoseKeys::GetKeyForSlot(Slot);
	Candidate.Hand = Hand;

	const FUxtTrackedHandFrame& HandFrame = GetCachedHandFrame(Hand);
	if (!HandFrame.bIsTracked || !ActiveBindingsAsset)
	{
		return Candidate;
	}

	const FUxtStaticHandPoseBinding* Binding = ActiveBindingsAsset->GetBinding(Slot);
	if (!Binding || !Binding->PoseDefinitionAsset)
	{
		return Candidate;
	}

	Candidate.Priority = Binding->Priority;
	Candidate.bValid = true;
	Candidate.bPassed = Binding->PoseDefinitionAsset->EvaluateHand(HandFrame, Candidate.Evaluation);
	Candidate.Score = Candidate.Evaluation.Score;

	const EUxtPalmCameraDirection PalmDirection = FUxtPalmDirectionUtility::GetPalmDirectionBit(HandFrame, CameraTransform);
	if (!FUxtPalmDirectionUtility::IsDirectionAllowed(Binding->AllowedPalmDirs, PalmDirection))
	{
		Candidate.bPassed = false;
		Candidate.Evaluation.bPassed = false;
		Candidate.Evaluation.FailureReason = EUxtStaticPoseFailureReason::PalmDirRejected;
		Candidate.Evaluation.Score = 0.0f;
		Candidate.Score = 0.0f;
	}

	return Candidate;
}

void UUxtStaticHandPoseSubsystem::UpdateBindingState(EUxtHandPoseKeySlot Slot, const FBindingCandidate& Candidate, float TimeSeconds)
{
	if (!ActiveBindingsAsset)
	{
		return;
	}

	FBindingEntry* Entry = BindingEntries.FindByPredicate(
		[Slot](const FBindingEntry& Item) { return Item.Slot == Slot; });
	if (!Entry)
	{
		return;
	}

	FBindingRuntimeState& RuntimeState = Entry->RuntimeState;
	FUxtStaticHandPoseBindingDebugInfo& DebugInfo = Entry->DebugInfo;
	const bool bWasPhysicallyPressed = RuntimeState.bPhysicallyPressed;
	const FUxtStaticHandPoseBinding* Binding = ActiveBindingsAsset->GetBinding(Slot);
	if (!Binding)
	{
		return;
	}

	DebugInfo.TriggeringHand = Candidate.Hand;
	DebugInfo.Score = Candidate.Score;
	DebugInfo.bPassed = Candidate.bPassed;
	DebugInfo.Evaluation = Candidate.Evaluation;

	const bool bCandidatePressed = Candidate.bValid && Candidate.bPassed;

	switch (RuntimeState.State)
	{
	case EUxtStaticHandPoseBindingState::Idle:
		if (bCandidatePressed)
		{
			RuntimeState.State = Binding->DebounceDownTime > 0.0f ? EUxtStaticHandPoseBindingState::CandidateDown :
				EUxtStaticHandPoseBindingState::Pressed;
			RuntimeState.StateChangeTime = TimeSeconds;
			RuntimeState.TriggeringHand = Candidate.Hand;
			if (RuntimeState.State == EUxtStaticHandPoseBindingState::Pressed)
			{
				RuntimeState.bPhysicallyPressed = true;
				RuntimeState.PressedStartTime = TimeSeconds;
			}
		}
		break;

	case EUxtStaticHandPoseBindingState::CandidateDown:
		if (!bCandidatePressed)
		{
			RuntimeState.State = EUxtStaticHandPoseBindingState::Idle;
			RuntimeState.TriggeringHand = EControllerHand::AnyHand;
		}
		else if ((TimeSeconds - RuntimeState.StateChangeTime) >= Binding->DebounceDownTime)
		{
			RuntimeState.State = EUxtStaticHandPoseBindingState::Pressed;
			RuntimeState.bPhysicallyPressed = true;
			RuntimeState.PressedStartTime = TimeSeconds;
		}
		break;

	case EUxtStaticHandPoseBindingState::Pressed:
		if (!bCandidatePressed)
		{
			RuntimeState.State = (Binding->DebounceUpTime > 0.0f || Binding->MinHoldTime > 0.0f) ?
				EUxtStaticHandPoseBindingState::CandidateUp : EUxtStaticHandPoseBindingState::Idle;
			RuntimeState.StateChangeTime = TimeSeconds;
			if (RuntimeState.State == EUxtStaticHandPoseBindingState::Idle)
			{
				RuntimeState.bPhysicallyPressed = false;
				RuntimeState.TriggeringHand = EControllerHand::AnyHand;
			}
		}
		else
		{
			RuntimeState.TriggeringHand = Candidate.Hand;
		}
		break;

	case EUxtStaticHandPoseBindingState::CandidateUp:
		if (bCandidatePressed)
		{
			RuntimeState.State = EUxtStaticHandPoseBindingState::Pressed;
			RuntimeState.TriggeringHand = Candidate.Hand;
		}
		else if ((TimeSeconds - RuntimeState.StateChangeTime) >= Binding->DebounceUpTime &&
			(TimeSeconds - RuntimeState.PressedStartTime) >= Binding->MinHoldTime)
		{
			RuntimeState.State = EUxtStaticHandPoseBindingState::Idle;
			RuntimeState.bPhysicallyPressed = false;
			RuntimeState.TriggeringHand = EControllerHand::AnyHand;
		}
		break;
	}

	DebugInfo.State = RuntimeState.State;
	DebugInfo.TriggeringHand = RuntimeState.TriggeringHand;
	if (bWasPhysicallyPressed != RuntimeState.bPhysicallyPressed)
	{
		OnKeyStateChange.Broadcast(Slot, RuntimeState.bPhysicallyPressed);
	}
	FUXToolsHandPoseModule::Get().SetKeyPressed(DebugInfo.Key, RuntimeState.bPhysicallyPressed);
}

void UUxtStaticHandPoseSubsystem::ClearModuleKeyStates() const
{
	FUXToolsHandPoseModule& Module = FUXToolsHandPoseModule::Get();
	for (const FKey& Key : UxtHandPoseKeys::GetAllKeys())
	{
		Module.SetKeyPressed(Key, false);
	}
}

bool UUxtStaticHandPoseSubsystem::GetCameraTransform(UWorld* World, FTransform& OutCameraTransform) const
{
	if (!World)
	{
		return false;
	}

	if (APlayerController* PlayerController = World->GetFirstPlayerController())
	{
		FVector ViewLocation;
		FRotator ViewRotation;
		PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
		OutCameraTransform = FTransform(ViewRotation, ViewLocation);
		return true;
	}

	return false;
}

const FUxtTrackedHandFrame& UUxtStaticHandPoseSubsystem::GetCachedHandFrame(EControllerHand Hand) const
{
	return Hand == EControllerHand::Right ? CachedRightHandFrame : CachedLeftHandFrame;
}

FUxtTrackedHandFrame& UUxtStaticHandPoseSubsystem::GetMutableCachedHandFrame(EControllerHand Hand)
{
	return Hand == EControllerHand::Right ? CachedRightHandFrame : CachedLeftHandFrame;
}
