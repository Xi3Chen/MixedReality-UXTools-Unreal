#include "UXToolsHandPose.h"

#include "GenericPlatform/GenericApplicationMessageHandler.h"
#include "IInputDevice.h"
#include "InputCoreTypes.h"

#define LOCTEXT_NAMESPACE "FUXToolsHandPoseModule"

namespace
{
	TArray<FKey> GHandPoseKeys;

	class FUXToolsHandPoseInputDevice : public IInputDevice
	{
	public:
		explicit FUXToolsHandPoseInputDevice(FUXToolsHandPoseModule& InModule)
			: Module(InModule)
			, MessageHandler(MakeShared<FGenericApplicationMessageHandler>())
		{
		}

		virtual void Tick(float DeltaTime) override
		{
		}

		virtual void SendControllerEvents() override
		{
			for (const FKey& Key : UxtHandPoseKeys::GetAllKeys())
			{
				const bool bPressed = Module.IsKeyPressed(Key);
				const bool bWasPressed = SentStates.FindRef(Key);

				if (bPressed && !bWasPressed)
				{
					SentStates.Add(Key, true);
					MessageHandler->OnControllerButtonPressed(Key.GetFName(), 0, false);
				}
				else if (!bPressed && bWasPressed)
				{
					SentStates.Add(Key, false);
					MessageHandler->OnControllerButtonReleased(Key.GetFName(), 0, false);
				}
			}
		}

		virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override
		{
			MessageHandler = InMessageHandler;
		}

		virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override
		{
			return false;
		}

		virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override
		{
		}

		virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& values) override
		{
		}

	private:
		FUXToolsHandPoseModule& Module;
		TSharedPtr<FGenericApplicationMessageHandler> MessageHandler;
		TMap<FKey, bool> SentStates;
	};
}

namespace UxtHandPoseKeys
{
	const FKey& GetKey(int32 Index)
	{
		return GHandPoseKeys[Index];
	}

	FKey GetKeyForSlot(EUxtHandPoseKeySlot Slot)
	{
		const int32 Index = static_cast<int32>(Slot) - 1;
		return GHandPoseKeys.IsValidIndex(Index) ? GHandPoseKeys[Index] : FKey();
	}

	const TArray<FKey>& GetAllKeys()
	{
		return GHandPoseKeys;
	}
}

FUXToolsHandPoseModule& FUXToolsHandPoseModule::Get()
{
	return FModuleManager::LoadModuleChecked<FUXToolsHandPoseModule>(TEXT("UXToolsHandPose"));
}

void FUXToolsHandPoseModule::StartupModule()
{
	IInputDeviceModule::StartupModule();
	RegisterKeys();
	InputDevice = MakeShared<FUXToolsHandPoseInputDevice>(*this);
}

void FUXToolsHandPoseModule::ShutdownModule()
{
	ClearKeyStates();
	InputDevice.Reset();
	UnregisterKeys();
}

TSharedPtr<IInputDevice> FUXToolsHandPoseModule::CreateInputDevice(
	const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	if (InputDevice.IsValid())
	{
		InputDevice->SetMessageHandler(InMessageHandler);
	}

	return InputDevice;
}

void FUXToolsHandPoseModule::SetKeyPressed(const FKey& Key, bool bPressed)
{
	if (Key.IsValid())
	{
		KeyStates.Add(Key, bPressed);
	}
}

bool FUXToolsHandPoseModule::IsKeyPressed(const FKey& Key) const
{
	return KeyStates.FindRef(Key);
}

void FUXToolsHandPoseModule::ClearKeyStates()
{
	for (const FKey& Key : UxtHandPoseKeys::GetAllKeys())
	{
		KeyStates.Add(Key, false);
	}
}

void FUXToolsHandPoseModule::RegisterKeys()
{
	if (!GHandPoseKeys.IsEmpty())
	{
		return;
	}

	EKeys::AddMenuCategoryDisplayInfo(
		"UXToolsHandPose", LOCTEXT("UXToolsHandPoseCategory", "UXTools Hand Pose"), TEXT("GraphEditor.PadEvent_16x"));

	for (int32 KeyIndex = 0; KeyIndex < 10; ++KeyIndex)
	{
		const FString KeyName = FString::Printf(TEXT("Custom_HandPoseKey_%d"), KeyIndex + 1);
		const FString DisplayName = FString::Printf(TEXT("Custom Hand Pose Key %d"), KeyIndex + 1);
		const FKey Key(*KeyName);
		GHandPoseKeys.Add(Key);
		EKeys::AddKey(FKeyDetails(
			Key, FText::FromString(DisplayName), FKeyDetails::GamepadKey,
			"UXToolsHandPose"));
		KeyStates.Add(Key, false);
	}
}

void FUXToolsHandPoseModule::UnregisterKeys()
{
	if (!GHandPoseKeys.IsEmpty())
	{
		EKeys::RemoveKeysWithCategory("UXToolsHandPose");
		GHandPoseKeys.Reset();
		KeyStates.Reset();
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FUXToolsHandPoseModule, UXToolsHandPose)
