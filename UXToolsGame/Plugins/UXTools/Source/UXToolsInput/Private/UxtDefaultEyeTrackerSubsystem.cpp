// Fill out your copyright notice in the Description page of Project Settings.


#include "UxtDefaultEyeTrackerSubsystem.h"

#include "EyeTracking/IUxtEyeTracker.h"
#include "Engine/World.h"
#include "Features/IModularFeatures.h"

void UUxtDefaultEyeTrackerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Map load/unload hooks. We register the eye tracker per world lifetime so that unloading a map
	// cleanly falls back to the HMD tracker (IUxtEyeTracker::GetHdmTracker).
	PostLoadMapHandle =
		FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UUxtDefaultEyeTrackerSubsystem::OnPostLoadMapWithWorld);
	WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &UUxtDefaultEyeTrackerSubsystem::OnWorldCleanup);

	// In case we initialize after a world already exists (e.g. PIE), attempt immediate registration.
	RegisterIfNeeded();
}

void UUxtDefaultEyeTrackerSubsystem::Deinitialize()
{
	Super::Deinitialize();
	UnregisterIfNeeded();

	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
	FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
	PostLoadMapHandle.Reset();
	WorldCleanupHandle.Reset();

}

void UUxtDefaultEyeTrackerSubsystem::RegisterIfNeeded()
{
	if (bRegistered)
	{
		return;
	}

	// If you later add a "device available" check, do it here.
	IModularFeatures::Get().RegisterModularFeature(IUxtEyeTracker::GetModularFeatureName(), &DefaultEyeTracker);
	bRegistered = true;
}

void UUxtDefaultEyeTrackerSubsystem::UnregisterIfNeeded()
{
	if (!bRegistered)
	{
		return;
	}

	IModularFeatures::Get().UnregisterModularFeature(IUxtEyeTracker::GetModularFeatureName(), &DefaultEyeTracker);
	bRegistered = false;
}

void UUxtDefaultEyeTrackerSubsystem::OnPostLoadMapWithWorld(UWorld* LoadedWorld)
{
	// Map loaded: ensure our default eye tracker is available.
	RegisterIfNeeded();
}

void UUxtDefaultEyeTrackerSubsystem::OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	// Map/world unloading: remove the modular feature so IUxtEyeTracker::Get() falls back to HMD tracker.
	UnregisterIfNeeded();
}

