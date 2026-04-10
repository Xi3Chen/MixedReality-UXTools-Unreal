// Fill out your copyright notice in the Description page of Project Settings.


#include "Controls/UxtUINodeController.h"


FUxtComponentInteractionData::FUxtComponentInteractionData(USceneComponent* InTargetComponent)
{
	TargetComponent = InTargetComponent;
	if(!IsValidInteractionData())
	{
		return;
	}
	if(InTargetComponent->GetClass()->IsChildOf(UPrimitiveComponent::StaticClass()))
	{
		bIsPrimitiveComponent = true;
		Collision = Cast<UPrimitiveComponent>(TargetComponent)->GetCollisionEnabled();
	}
	else if(InTargetComponent->GetClass()->IsChildOf(UChildActorComponent::StaticClass()))
	{
		bIsActorComponent = true;
	}
	bIsVisible =InTargetComponent->IsVisible();
	bIsActor = InTargetComponent->GetOwner()->GetRootComponent() == InTargetComponent;
}

bool FUxtComponentInteractionData::IsValidInteractionData() const
{
	return TargetComponent && IsValid(TargetComponent);
}

// Sets default values for this component's properties
UUxtUINodeController::UUxtUINodeController()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UUxtUINodeController::CollectComponentInteractionData()
{
	UxtComponentInteractionDataSet.Empty();
	auto Child = this->GetAttachChildren();
	for (auto Component : Child)
	{
		UxtComponentInteractionDataSet.Add(FUxtComponentInteractionData(Component));
	}
	
}

void UUxtUINodeController::SetUIVisibility(EUxtUIElementVisibility NewVisibility)
{
	Visibility = NewVisibility;
	auto SetUxtActorUIVisibility = [](AActor* TargetActor, EUxtUIElementVisibility NewVisibility)
	{
		if(!TargetActor)
		{
			return;
		}
		UUxtUIElementComponent* UxtComponent = TargetActor->FindComponentByClass<UUxtUIElementComponent>();
		if (UxtComponent)
		{
			UxtComponent->SetUIVisibility(NewVisibility);
		}
	};
	for(auto it:UxtComponentInteractionDataSet)
	{
		USceneComponent* TargetComponent = it.TargetComponent;
		if(!it.IsValidInteractionData())
		{
			check(0);
			continue;
		}
		if(it.bIsActor)
		{
			SetUxtActorUIVisibility(TargetComponent->GetOwner(),NewVisibility);
			continue;
		}
		if(it.bIsActorComponent)
		{
			if(UChildActorComponent* ActorComponent = Cast<UChildActorComponent>(TargetComponent))
			{
				SetUxtActorUIVisibility(ActorComponent->GetChildActor(),NewVisibility);
			}
			continue;
		}
		if(it.bIsPrimitiveComponent)
		{
			Cast<UPrimitiveComponent>(it.TargetComponent)->SetCollisionEnabled(NewVisibility==EUxtUIElementVisibility::Show?it.Collision.GetValue():ECollisionEnabled::NoCollision);
			
		}
		TargetComponent->SetVisibility(NewVisibility==EUxtUIElementVisibility::Show,true);
	}
}


// Called when the game starts
void UUxtUINodeController::BeginPlay()
{
	Super::BeginPlay();

	// ...
	CollectComponentInteractionData();
	if(Visibility != EUxtUIElementVisibility::Show)
	{
		SetUIVisibility(Visibility);
	}
}


// Called every frame
void UUxtUINodeController::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
