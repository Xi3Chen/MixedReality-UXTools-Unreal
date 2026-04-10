#include "UxtStaticHandPoseWorldSubsystem.h"

#include "Engine/Engine.h"
#include "UxtStaticHandPoseSubsystem.h"

void UUxtStaticHandPoseWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (GEngine)
	{
		if (UUxtStaticHandPoseSubsystem* HandPoseSubsystem = GEngine->GetEngineSubsystem<UUxtStaticHandPoseSubsystem>())
		{
			HandPoseSubsystem->ReloadBindings();
		}
	}
}
