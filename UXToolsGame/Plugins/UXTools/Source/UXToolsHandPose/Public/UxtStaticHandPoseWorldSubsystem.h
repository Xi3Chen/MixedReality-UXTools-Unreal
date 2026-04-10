#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "UxtStaticHandPoseWorldSubsystem.generated.h"

/**
 * Reloads static hand pose bindings when a world subsystem is initialized,
 * so the engine subsystem picks up the latest project settings on world startup.
 */
UCLASS()
class UXTOOLSHANDPOSE_API UUxtStaticHandPoseWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};
