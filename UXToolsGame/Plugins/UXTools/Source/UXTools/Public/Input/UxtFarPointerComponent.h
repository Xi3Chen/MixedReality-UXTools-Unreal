// Copyright (c) 2020 Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "EngineDefines.h"
#include "InputCoreTypes.h"
#include "UxtPointerComponent.h"


#include "Components/ActorComponent.h"
#include "Materials/MaterialParameterCollection.h"

#include "UxtFarPointerComponent.generated.h"

class UUxtFarPointerComponent;
class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUxtFarPointerEnabledDelegate, UUxtFarPointerComponent*, FarPointer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUxtFarPointerDisabledDelegate, UUxtFarPointerComponent*, FarPointer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams (FUxtOnFarPointerPoseUpdated, UUxtFarPointerComponent*,FarPointer,FVector,PointerOrigin,FQuat,Orientation);

/**
 * Component that casts a ray for the given hand-tracked hand and raises far interaction events on the far targets hit.
 * A far target is an actor or component implementing the UUxtFarTarget interface.
 */
UCLASS(ClassGroup = "UXTools", meta = (BlueprintSpawnableComponent))
class UXTOOLS_API UUxtFarPointerComponent : public UUxtPointerComponent
{
	GENERATED_BODY()

public:
	UUxtFarPointerComponent();

	/** Origin of the pointer ray as reported by the hand tracker. See GetRayStart() for actual start of the ray used for querying the
	 * scene. */
	UFUNCTION(BlueprintCallable, Category = "Uxt Far Pointer")
	FVector GetPointerOrigin() const;

	/** Orientation of the pointer ray. */
	UFUNCTION(BlueprintCallable, Category = "Uxt Far Pointer")
	FQuat GetPointerOrientation() const;

	/** Start of the ray used for querying the scene. This is the pointer origin shifted by the ray start offset in the pointer forward
	 * direction. */
	UFUNCTION(BlueprintCallable, Category = "Uxt Far Pointer")
	FVector GetRayStart() const;

	/** Primitive the pointer is currently hitting or null if none. */
	UFUNCTION(BlueprintCallable, Category = "Uxt Far Pointer")
	UPrimitiveComponent* GetHitPrimitive() const;

	/** Current hit point location or ray end if there's no hit. */
	UFUNCTION(BlueprintCallable, Category = "Uxt Far Pointer")
	FVector GetHitPoint() const;

	/** Current hit point normal or negative ray direction if there's no hit. */
	UFUNCTION(BlueprintCallable, Category = "Uxt Far Pointer")
	FVector GetHitNormal() const;

	/** Whether the pointer is currently pressed. */
	UFUNCTION(BlueprintCallable, Category = "Uxt Far Pointer")
	bool IsPressed() const;

	/** Whether the pointer is currently enabled. Hit information is only valid while the pointer is enabled. */
	UFUNCTION(BlueprintCallable, Category = "Uxt Far Pointer")
	bool IsEnabled() const;

	//
	// UActorComponent interface

	virtual void SetActive(bool bNewActive, bool bReset = false) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//
	// UUxtPointerComponent interface

	virtual void SetFocusLocked(bool bLocked) override;
	virtual UObject* GetFocusTarget() const override;
	virtual FTransform GetCursorTransform() const override;

protected:
	/** Called every tick to update the pointer pose with the latest information from the hand tracker. */
	virtual void OnPointerPoseUpdated(const FQuat& NewOrientation, const FVector& NewOrigin);
	FVector ComputeRayPivotPosition(FVector handPosition, FTransform headTransform, const EControllerHand DeviceHand) const;
	/** Called every tick to update the pressed state with the latest information from the hand tracker. */
	void SetPressed(bool bNewPressed);

	/** Used to enable/disable the pointer on tracking gain/loss or component activation/deactivation. */
	void SetEnabled(bool bNewEnabled);

	/** Current far target if any. This will be a UObject implementing the UUxtFarTarget interface. */
	UObject* GetFarTarget() const;

#if ENABLE_VISUAL_LOG
	void VLogPointer() const;
#endif // ENABLE_VISUAL_LOG

public:
	/** Trace channel to be used in the pointer's line trace query. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Uxt Far Pointer")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECollisionChannel::ECC_Visibility;

	/** Start of the pointer ray expressed as an offset from the hand ray origin in the ray direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Uxt Far Pointer")
	float RayStartOffset = 5;

	/** Pointer ray length from ray start. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Uxt Far Pointer")
	float RayLength = 500;

	UPROPERTY(BlueprintAssignable, Category = "Uxt Far Pointer")
	FUxtFarPointerEnabledDelegate OnFarPointerEnabled;

	UPROPERTY(BlueprintAssignable, Category = "Uxt Far Pointer")
	FUxtFarPointerDisabledDelegate OnFarPointerDisabled;

	UPROPERTY(BlueprintAssignable, Category = "Uxt Far Pointer")
	FUxtOnFarPointerPoseUpdated FOnFarPointerPoseUpdated;
	
protected:
	void UpdateParameterCollection(FVector IndexTipPosition);

	/** Parameter collection used to store the finger tip position */
	UPROPERTY(Transient)
	UMaterialParameterCollection* ParameterCollection;

	/** Pointer origin as reported by the hand tracker. */
	FVector PointerOrigin = FVector::ZeroVector;

	/** Pointer orientation. */
	FQuat PointerOrientation = FQuat::Identity;

	TWeakObjectPtr<UPrimitiveComponent> HitPrimitiveWeak;
	FVector HitPoint = FVector::ZeroVector;
	FVector HitNormal = FVector::BackwardVector;

	// Hit information in the primitive space. Used to keep the hit relative to the primitive while the pointer is locked.
	FVector HitPointLocal = FVector::ZeroVector;
	FVector HitNormalLocal = FVector::BackwardVector;

	/** Far target that owns the hit primitive, if any. */
	TWeakObjectPtr<UObject> FarTargetWeak;
	// Constants from Shell Implementation of hand ray.
	float DynamicPivotBaseY = -10.0f, DynamicPivotMultiplierY = 0.65f, DynamicPivotMinY = -60.0f, DynamicPivotMaxY = -20.0f;
	float DynamicPivotBaseX = 3.0f, DynamicPivotMultiplierX = 0.65f, DynamicPivotMinX = 8.0f, DynamicPivotMaxX = 15.0f;
	float HeadToPivotOffsetZ = 8.0f;
	float CursorBeamBackwardTolerance = 0.5f;
	float CursorBeamUpTolerance = 0.8f;
	bool bPressed = false;

	bool bEnabled = false;
};
