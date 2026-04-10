// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UxtDefaultEyeTracker.h"
#include "Subsystems/EngineSubsystem.h"
#include "Engine/EngineBaseTypes.h"
#include "UxtDefaultEyeTrackerSubsystem.generated.h"
/**
 * 
 */
UCLASS()
class UXTOOLSINPUT_API UUxtDefaultEyeTrackerSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	void RegisterIfNeeded();
	void UnregisterIfNeeded();

	void OnPostLoadMapWithWorld(UWorld* LoadedWorld);
	void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

private:
	FUxtDefaultEyeTracker DefaultEyeTracker;
	bool bRegistered = false;

	FDelegateHandle PostLoadMapHandle;
	FDelegateHandle WorldCleanupHandle;
};
