#include "UxtHandPoseAxisActor.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "HeadMountedDisplayTypes.h"
#include "IXRTrackingSystem.h"
#include "UxtStaticHandPoseSubsystem.h"
#include "UxtStaticHandPoseUtilities.h"

namespace
{
	UArrowComponent* CreateArrow(
		const FObjectInitializer& ObjectInitializer,
		AActor* Owner,
		USceneComponent* Parent,
		const TCHAR* Name,
		const FRotator& RelativeRotation,
		const FLinearColor& Color,
		float ArrowLength)
	{
		UArrowComponent* Arrow = ObjectInitializer.CreateDefaultSubobject<UArrowComponent>(Owner, Name);
		Arrow->SetupAttachment(Parent);
		Arrow->SetUsingAbsoluteScale(false);
		Arrow->SetHiddenInGame(true);
		Arrow->SetVisibility(true);
		Arrow->ArrowSize = 0.25f;
		Arrow->ArrowLength = ArrowLength;
		Arrow->SetArrowColor(Color.ToFColor(true));
		Arrow->SetRelativeRotation(RelativeRotation);
		return Arrow;
	}
}

AUxtHandPoseAxisActor::AUxtHandPoseAxisActor(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));
	SetRootComponent(SceneRoot);

	UpArrow = CreateArrow(ObjectInitializer, this, SceneRoot, TEXT("UpArrow"), FRotator(90.0f, 0.0f, 0.0f), InactiveColor, ArrowLength);
	DownArrow = CreateArrow(ObjectInitializer, this, SceneRoot, TEXT("DownArrow"), FRotator(-90.0f, 0.0f, 0.0f), InactiveColor, ArrowLength);
	LeftArrow = CreateArrow(ObjectInitializer, this, SceneRoot, TEXT("LeftArrow"), FRotator(0.0f, -90.0f, 0.0f), InactiveColor, ArrowLength);
	RightArrow = CreateArrow(ObjectInitializer, this, SceneRoot, TEXT("RightArrow"), FRotator(0.0f, 90.0f, 0.0f), InactiveColor, ArrowLength);
	ForwardArrow = CreateArrow(ObjectInitializer, this, SceneRoot, TEXT("ForwardArrow"), FRotator::ZeroRotator, InactiveColor, ArrowLength);
	BackwardArrow = CreateArrow(ObjectInitializer, this, SceneRoot, TEXT("BackwardArrow"), FRotator(0.0f, 180.0f, 0.0f), InactiveColor, ArrowLength);

	SetActorHiddenInGame(true);
}

void AUxtHandPoseAxisActor::BeginPlay()
{
	Super::BeginPlay();

	UpdateArrowLength();
	SetAllArrowColors(InactiveColor);
	SetAllArrowsHidden(true);
	SetActorTickEnabled(false);
	SetActorHiddenInGame(true);

	if (UWorld* World = GetWorld())
	{
		if (UUxtStaticHandPoseSubsystem* Subsystem = GEngine ? GEngine->GetEngineSubsystem<UUxtStaticHandPoseSubsystem>() : nullptr)
		{
			HandPoseSubsystem = Subsystem;
			Subsystem->OnKeyStateChange.AddDynamic(this, &AUxtHandPoseAxisActor::HandleKeyStateChanged);
		}
	}
}

void AUxtHandPoseAxisActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UUxtStaticHandPoseSubsystem* Subsystem = HandPoseSubsystem.Get())
	{
		Subsystem->OnKeyStateChange.RemoveDynamic(this, &AUxtHandPoseAxisActor::HandleKeyStateChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void AUxtHandPoseAxisActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bTrackingActive)
	{
		return;
	}

	const EControllerHand Hand = bTrackedHandIsLeft ? EControllerHand::Left : EControllerHand::Right;
	FVector CurrentPalmTrackingLocation;
	if (!TryGetTrackingSpacePalmLocation(Hand, CurrentPalmTrackingLocation))
	{
		UpdateActiveDirection(EUxtHandPoseAxisDirection::None, false);
		return;
	}

	const FVector TrackingSpaceDelta = CurrentPalmTrackingLocation - StartPalmTrackingLocation;
	const bool bPassedThreshold = TrackingSpaceDelta.Size() >= MovementThresholdCm;
	const EUxtHandPoseAxisDirection NewDirection =
		bPassedThreshold ? GetDirectionFromDelta(TrackingSpaceDelta) : EUxtHandPoseAxisDirection::None;

	UpdateActiveDirection(NewDirection, false);
	UpdateActorTransformFromTrackingSpace();
}

void AUxtHandPoseAxisActor::HandleKeyStateChanged(EUxtHandPoseKeySlot InSlot, bool bIsPressed, bool bIsLeftHand)
{
	if (InSlot != Slot)
	{
		return;
	}

	bTrackedHandIsLeft = bIsLeftHand;

	if (!bIsPressed)
	{
		BroadcastDeactivateForAllAxes();
		ActiveDirection = EUxtHandPoseAxisDirection::None;
		ApplyArrowColors(EUxtHandPoseAxisDirection::None);
		SetActorActiveState(false);
		bTrackingActive = false;
		return;
	}

	const EControllerHand Hand = bTrackedHandIsLeft ? EControllerHand::Left : EControllerHand::Right;
	if (!TryGetTrackingSpacePalmLocation(Hand, StartPalmTrackingLocation))
	{
		return;
	}

	bTrackingActive = true;
	UpdateActiveDirection(EUxtHandPoseAxisDirection::None, true);
	UpdateActorTransformFromTrackingSpace();
	SetActorActiveState(true);
}

bool AUxtHandPoseAxisActor::TryGetTrackingSpacePalmLocation(EControllerHand Hand, FVector& OutPalmLocation) const
{
	OutPalmLocation = FVector::ZeroVector;

	UWorld* World = GetWorld();
	if (!World || !GEngine)
	{
		return false;
	}

	IXRTrackingSystem* XRSystem = GEngine->XRSystem.Get();
	if (!XRSystem)
	{
		return false;
	}

	const FTransform IdentityTrackingToWorld(FRotator::ZeroRotator, FVector::ZeroVector);
	const FTransform OldTrackingToWorld = XRSystem->GetTrackingToWorldTransform();
	XRSystem->UpdateTrackingToWorldTransform(IdentityTrackingToWorld);

	FXRMotionControllerData MotionControllerData;
	XRSystem->GetMotionControllerData(World, Hand, MotionControllerData);

	FUxtTrackedHandFrame HandFrame;
	const bool bBuiltFrame = FUxtPalmDirectionUtility::BuildTrackedHandFrame(
		MotionControllerData,
		IdentityTrackingToWorld,
		GFrameCounter,
		World->GetTimeSeconds(),
		HandFrame);

	XRSystem->UpdateTrackingToWorldTransform(OldTrackingToWorld);

	if (!bBuiltFrame || !HandFrame.bIsTracked)
	{
		return false;
	}

	FTransform PalmTransform;
	if (!HandFrame.GetJointWorldTransform(EHandKeypoint::Palm, PalmTransform))
	{
		return false;
	}

	OutPalmLocation = PalmTransform.GetLocation();
	return true;
}

bool AUxtHandPoseAxisActor::TryGetTrackingToWorldTransform(FTransform& OutTrackingToWorld) const
{
	if (!GEngine)
	{
		return false;
	}

	IXRTrackingSystem* XRSystem = GEngine->XRSystem.Get();
	if (!XRSystem)
	{
		return false;
	}

	OutTrackingToWorld = XRSystem->GetTrackingToWorldTransform();
	return true;
}

void AUxtHandPoseAxisActor::UpdateActorTransformFromTrackingSpace()
{
	FTransform TrackingToWorld;
	if (!TryGetTrackingToWorldTransform(TrackingToWorld))
	{
		return;
	}

	const FTransform TrackingSpaceTransform(FQuat::Identity, StartPalmTrackingLocation);
	const FTransform WorldTransform = TrackingSpaceTransform * TrackingToWorld;
	SetActorTransform(WorldTransform);
}

void AUxtHandPoseAxisActor::SetActorActiveState(bool bInActive)
{
	SetActorHiddenInGame(!bInActive);
	SetActorTickEnabled(bInActive);
	SetAllArrowsHidden(!bInActive);
	if (!bInActive)
	{
		SetAllArrowColors(InactiveColor);
	}
}

void AUxtHandPoseAxisActor::SetAllArrowsHidden(bool bShouldHide)
{
	for (UArrowComponent* Arrow : GetArrowComponents())
	{
		if (Arrow)
		{
			Arrow->SetHiddenInGame(bShouldHide);
		}
	}
}

void AUxtHandPoseAxisActor::UpdateArrowLength()
{
	for (UArrowComponent* Arrow : GetArrowComponents())
	{
		if (Arrow)
		{
			Arrow->ArrowLength = ArrowLength;
		}
	}
}

void AUxtHandPoseAxisActor::SetAllArrowColors(const FLinearColor& Color)
{
	const FColor ArrowColor = Color.ToFColor(true);
	for (UArrowComponent* Arrow : GetArrowComponents())
	{
		if (Arrow)
		{
			Arrow->SetArrowColor(ArrowColor);
		}
	}
}

void AUxtHandPoseAxisActor::UpdateActiveDirection(EUxtHandPoseAxisDirection NewDirection, bool bForceDeactivateAll)
{
	if (bForceDeactivateAll)
	{
		ActiveDirection = EUxtHandPoseAxisDirection::None;
		BroadcastDeactivateForAllAxes();
	}

	if (ActiveDirection == NewDirection)
	{
		ApplyArrowColors(NewDirection);
		return;
	}

	if (ActiveDirection != EUxtHandPoseAxisDirection::None)
	{
		OnAxisDeactivated.Broadcast(ActiveDirection);
	}

	ActiveDirection = NewDirection;

	if (ActiveDirection != EUxtHandPoseAxisDirection::None)
	{
		OnAxisActivated.Broadcast(ActiveDirection);
	}

	ApplyArrowColors(ActiveDirection);
}

void AUxtHandPoseAxisActor::ApplyArrowColors(EUxtHandPoseAxisDirection Direction)
{
	SetAllArrowColors(InactiveColor);
	if (UArrowComponent* Arrow = GetArrowForDirection(Direction))
	{
		Arrow->SetArrowColor(ActiveColor.ToFColor(true));
	}
}

void AUxtHandPoseAxisActor::BroadcastDeactivateForAllAxes()
{
	static const EUxtHandPoseAxisDirection Directions[] = {
		EUxtHandPoseAxisDirection::Up,
		EUxtHandPoseAxisDirection::Down,
		EUxtHandPoseAxisDirection::Left,
		EUxtHandPoseAxisDirection::Right,
		EUxtHandPoseAxisDirection::Forward,
		EUxtHandPoseAxisDirection::Backward,
	};

	for (const EUxtHandPoseAxisDirection Direction : Directions)
	{
		OnAxisDeactivated.Broadcast(Direction);
	}
}

TArray<UArrowComponent*> AUxtHandPoseAxisActor::GetArrowComponents() const
{
	return {UpArrow, DownArrow, LeftArrow, RightArrow, ForwardArrow, BackwardArrow};
}

UArrowComponent* AUxtHandPoseAxisActor::GetArrowForDirection(EUxtHandPoseAxisDirection Direction) const
{
	switch (Direction)
	{
	case EUxtHandPoseAxisDirection::Up:
		return UpArrow;
	case EUxtHandPoseAxisDirection::Down:
		return DownArrow;
	case EUxtHandPoseAxisDirection::Left:
		return LeftArrow;
	case EUxtHandPoseAxisDirection::Right:
		return RightArrow;
	case EUxtHandPoseAxisDirection::Forward:
		return ForwardArrow;
	case EUxtHandPoseAxisDirection::Backward:
		return BackwardArrow;
	default:
		return nullptr;
	}
}

EUxtHandPoseAxisDirection AUxtHandPoseAxisActor::GetDirectionFromDelta(const FVector& TrackingSpaceDelta) const
{
	const FVector AbsDelta(FMath::Abs(TrackingSpaceDelta.X), FMath::Abs(TrackingSpaceDelta.Y), FMath::Abs(TrackingSpaceDelta.Z));
	if (AbsDelta.X >= AbsDelta.Y && AbsDelta.X >= AbsDelta.Z)
	{
		return TrackingSpaceDelta.X >= 0.0f ? EUxtHandPoseAxisDirection::Forward : EUxtHandPoseAxisDirection::Backward;
	}

	if (AbsDelta.Y >= AbsDelta.Z)
	{
		return TrackingSpaceDelta.Y >= 0.0f ? EUxtHandPoseAxisDirection::Right : EUxtHandPoseAxisDirection::Left;
	}

	return TrackingSpaceDelta.Z >= 0.0f ? EUxtHandPoseAxisDirection::Up : EUxtHandPoseAxisDirection::Down;
}
