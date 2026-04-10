// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "UxtTrackingControllerSubsystem.generated.h"

class IHandTracker;

// 当玩家跟随相机组件更新时触发
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerFollowCameraUpdated, UCameraComponent*, NewCameraComponent);
struct FXRMotionControllerData;

class UCameraComponent;
/**
 * 
 */
UCLASS()
class UXTOOLS_API UUxtTrackingControllerSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	// 当玩家跟随相机组件更新时触发的事件
	UPROPERTY(BlueprintAssignable, Category = "UxtHandTracker")
	FOnPlayerFollowCameraUpdated OnPlayerFollowCameraUpdated;
	
	UFUNCTION(BlueprintPure,Category="UxtHandTracker")
	UCameraComponent* GetPlayerFollowCameraComponent()const;
	
	UFUNCTION(BlueprintCallable,Category="UxtHandTracker")
	void SetPlayerFollowCameraComponent(UCameraComponent* NewPlayerFollowCameraComponent);

	UFUNCTION(BlueprintCallable,Category="UxtHandTracker",meta=(WorldContext="WorldContextObject"))
	void GetXRMotionControllerData(UObject* WorldContextObject,EControllerHand Hand,bool bUsinWorldSpace,FXRMotionControllerData& MotionControllerData);
	UFUNCTION(BlueprintCallable, Category="UxtHandTracker", meta=(WorldContext="WorldContextObject"))
	FTransform ComputeTrackingToWorldTransform(UObject* WorldContextObject) const;


private:
	UPROPERTY(Transient)
	mutable UCameraComponent* PlayerFollowCameraComponent = nullptr;
	
};
