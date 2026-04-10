// Copyright (c) 2020 Microsoft Corporation.
// Licensed under the MIT License.

#include "Controls/UxtFarCursorComponent.h"

#include "UXTools.h"

#include "GameFramework/Actor.h"
#include "Input/UxtEyePointerComponent.h"
#include "Input/UxtFarPointerComponent.h"
#include "Utils/UxtFunctionLibrary.h"

UUxtFarCursorComponent::UUxtFarCursorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Will start ticking when the far pointer is enabled
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_LastDemotable;
	SetHiddenInGame(true);
}

void UUxtFarCursorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AActor* const Owner = GetOwner())
	{
		TArray<UUxtFarPointerComponent*> OwnerFarPointers;
		Owner->GetComponents<UUxtFarPointerComponent>(OwnerFarPointers);
		if (OwnerFarPointers.Num() > 0)
		{
			for (UUxtFarPointerComponent* FarPointer : OwnerFarPointers)
			{
				FarPointers.Add(FarPointer);

				// Tick after every pointer so we always render the latest active one.
				AddTickPrerequisiteComponent(FarPointer);

				FarPointer->OnFarPointerEnabled.AddDynamic(this, &UUxtFarCursorComponent::OnFarPointerEnabled);
				FarPointer->OnFarPointerDisabled.AddDynamic(this, &UUxtFarCursorComponent::OnFarPointerDisabled);
			}

			RefreshActiveFarPointer();
		}
		else
		{
			UE_LOG(UXTools, Error, TEXT("Could not find a far pointer in actor '%s'. Far cursor won't work properly."), *Owner->GetName());
		}
	}
}

void UUxtFarCursorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	for (const TWeakObjectPtr<UUxtFarPointerComponent>& FarPointerWeakEntry : FarPointers)
	{
		if (UUxtFarPointerComponent* FarPointer = FarPointerWeakEntry.Get())
		{
			// For extra safety check if the functions are bound prior to removing them.
			if (FarPointer->OnFarPointerEnabled.IsAlreadyBound(this, &UUxtFarCursorComponent::OnFarPointerEnabled))
			{
				FarPointer->OnFarPointerEnabled.RemoveDynamic(this, &UUxtFarCursorComponent::OnFarPointerEnabled);
			}
			if (FarPointer->OnFarPointerDisabled.IsAlreadyBound(this, &UUxtFarCursorComponent::OnFarPointerDisabled))
			{
				FarPointer->OnFarPointerDisabled.RemoveDynamic(this, &UUxtFarCursorComponent::OnFarPointerDisabled);
			}
		}
	}
}

void UUxtFarCursorComponent::RefreshActiveFarPointer()
{
	for (const TWeakObjectPtr<UUxtFarPointerComponent>& FarPointerWeakEntry : FarPointers)
	{
		if (UUxtFarPointerComponent* FarPointer = FarPointerWeakEntry.Get())
		{
			if (FarPointer->IsEnabled())
			{
				FarPointerWeak = FarPointer;
				SetVisualEnabled(true);
				return;
			}
		}
	}

	FarPointerWeak = nullptr;
	SetVisualEnabled(false);
}

void UUxtFarCursorComponent::SetVisualEnabled(bool bEnabled)
{
	SetActive(bEnabled);
	SetHiddenInGame(!bEnabled);
	if (!bEnabled)
	{
		SetPressed(false);
	}
}

void UUxtFarCursorComponent::OnFarPointerEnabled(UUxtFarPointerComponent* FarPointer)
{
	FarPointerWeak = FarPointer;
	SetVisualEnabled(true);
}

void UUxtFarCursorComponent::OnFarPointerDisabled(UUxtFarPointerComponent* FarPointer)
{
	if (FarPointerWeak.Get() == FarPointer)
	{
		RefreshActiveFarPointer();
	}
}

void UUxtFarCursorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (UUxtFarPointerComponent* FarPointer = FarPointerWeak.Get())
	{
		// Place hovering the hit location
		const FVector& HitNormal = FarPointer->GetHitNormal();
		FVector Location = FarPointer->GetHitPoint() + HitNormal * HoverDistance;
		if(Location.ContainsNaN())
		{
			return;
		}
		SetWorldLocation(Location);

		// Align with hit normal
		const FMatrix Rotation = FRotationMatrix::MakeFromX(-HitNormal);
		SetWorldRotation(Rotation.ToQuat());

		// Update pressed state
		SetPressed(FarPointer->IsPressed());

		// Scale with distance to head
		float DistanceToCamera = (UUxtFunctionLibrary::GetHeadPose(this).GetTranslation() - Location).Size();
		float ReferenceDistance = 100.0f;
		float NewRadius = bPressed ? PressedRadius : IdleRadius;
		if (Cast<UUxtEyePointerComponent>(FarPointer) != nullptr)
		{
			NewRadius *= 3.0f;
		}
		NewRadius *= DistanceToCamera / ReferenceDistance;
		SetRadius(NewRadius);
	}
}

void UUxtFarCursorComponent::SetPressed(bool bNewPressed)
{
	if (bNewPressed != bPressed)
	{
		bPressed = bNewPressed;

		if (bPressed)
		{
			SetStaticMesh(PressMesh);
		}
		else
		{
			SetStaticMesh(FocusMesh);
		}
	}
}
