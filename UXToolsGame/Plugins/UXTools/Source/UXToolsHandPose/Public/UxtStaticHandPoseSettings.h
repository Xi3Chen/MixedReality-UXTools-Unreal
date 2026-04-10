#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "UxtStaticHandPoseSettings.generated.h"

class UUxtStaticHandPoseBindingsAsset;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "UXTools Static Hand Pose"))
class UXTOOLSHANDPOSE_API UUxtStaticHandPoseSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Bindings")
	TSoftObjectPtr<UUxtStaticHandPoseBindingsAsset> BindingsAsset;
};
