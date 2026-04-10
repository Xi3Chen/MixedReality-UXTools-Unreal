#pragma once

#include "CoreMinimal.h"
#include "IInputDeviceModule.h"
#include "InputCoreTypes.h"

#include "UxtStaticHandPoseBindings.h"

#include "Modules/ModuleManager.h"

class FGenericApplicationMessageHandler;
class IInputDevice;

namespace UxtHandPoseKeys
{
	UXTOOLSHANDPOSE_API const FKey& GetKey(int32 Index);
	UXTOOLSHANDPOSE_API FKey GetKeyForSlot(EUxtHandPoseKeySlot Slot);
	UXTOOLSHANDPOSE_API const TArray<FKey>& GetAllKeys();
}

class FUXToolsHandPoseModule : public IInputDeviceModule
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
	virtual TSharedPtr<class IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;

	static FUXToolsHandPoseModule& Get();
	void SetKeyPressed(const FKey& Key, bool bPressed);
	bool IsKeyPressed(const FKey& Key) const;
	void ClearKeyStates();

private:
	void RegisterKeys();
	void UnregisterKeys();

	TSharedPtr<IInputDevice> InputDevice;
	TMap<FKey, bool> KeyStates;
};
