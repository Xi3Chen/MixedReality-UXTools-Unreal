// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UxtFarPointerComponent.h"
#include "UxtEyePointerComponent.generated.h"






UCLASS(ClassGroup="UXTools", meta=(BlueprintSpawnableComponent))
class UXTOOLS_API UUxtEyePointerComponent : public UUxtFarPointerComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UUxtEyePointerComponent();

	/** If true, bypass any registered eye tracker and always use the HMD fallback tracker. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Uxt Eye Pointer")
	bool bForceHmdTracker = false;

	//
	// UActorComponent interface
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	/** Called when the pointer pose is updated with the latest eye gaze information. */
	virtual void OnPointerPoseUpdated(const FQuat& NewOrientation, const FVector& NewOrigin) override;

	
};
