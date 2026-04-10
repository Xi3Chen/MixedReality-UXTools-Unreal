// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UxtUIElementComponent.h"
#include "Components/SceneComponent.h"
#include "UxtUINodeController.generated.h"


USTRUCT()
struct UXTOOLS_API FUxtComponentInteractionData
{
	GENERATED_BODY()
	FUxtComponentInteractionData() = default;
	FUxtComponentInteractionData(USceneComponent* InTargetComponent);
	bool bIsVisible = false;
	UPROPERTY()
	bool bIsPrimitiveComponent=false;
	UPROPERTY()
	bool bIsActorComponent=false;
	UPROPERTY()
	bool bIsActor = false;
	UPROPERTY()
	TEnumAsByte<ECollisionEnabled::Type> Collision = ECollisionEnabled::NoCollision;
	UPROPERTY()
	USceneComponent* TargetComponent = nullptr;

	bool IsValidInteractionData()const;
	// friend uint32 GetTypeHash(const FUxtComponentInteractionData& UxtComponentInteractionData);
	bool operator==(const FUxtComponentInteractionData& Target) const
	{
		return this->TargetComponent == Target.TargetComponent;
	}
	
};

FORCEINLINE uint32 GetTypeHash(const FUxtComponentInteractionData& UxtComponentInteractionData)
{
	return GetTypeHash(UxtComponentInteractionData.TargetComponent);
}

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UXTOOLS_API UUxtUINodeController : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UUxtUINodeController();
	//Collects interaction data between game components, aiding in analysis and debugging.
	UFUNCTION(BlueprintCallable, Category = "Uxt UI Element")
	void CollectComponentInteractionData();
	/** Set the element's visibility. The element will not be visible in the scene if it's parent is hidden. */
	UFUNCTION(BlueprintCallable, Category = "Uxt UI Element", DisplayName = "Set UI Visibility")
	void SetUIVisibility(EUxtUIElementVisibility NewVisibility);
public:
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
private:
	/** The element's visibility. */
	UPROPERTY(EditAnywhere, Category = "Uxt UI Element", DisplayName = "UI Visibility")
	EUxtUIElementVisibility Visibility = EUxtUIElementVisibility::Show;

	UPROPERTY(Transient)
	TSet<FUxtComponentInteractionData> UxtComponentInteractionDataSet;
};
