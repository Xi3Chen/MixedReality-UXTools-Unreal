#include "UXToolsHandPoseEditor.h"

#include "PropertyEditorModule.h"
#include "UxtStaticHandPoseCapturePanel.h"
#include "UxtStaticHandPoseCaptureTool.h"
#include "UxtStaticHandPoseCaptureToolCustomization.h"
#include "UxtStaticHandPoseDebugPanel.h"

#include "Framework/Docking/TabManager.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FUXToolsHandPoseEditorModule"

const FName FUXToolsHandPoseEditorModule::DebugTabName(TEXT("UXToolsHandPoseDebug"));
const FName FUXToolsHandPoseEditorModule::CaptureTabName(TEXT("UXToolsHandPoseCapture"));

void FUXToolsHandPoseEditorModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		DebugTabName,
		FOnSpawnTab::CreateRaw(this, &FUXToolsHandPoseEditorModule::SpawnDebugTab))
		.SetDisplayName(LOCTEXT("HandPoseDebugTabTitle", "Hand Pose Debug"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		CaptureTabName,
		FOnSpawnTab::CreateRaw(this, &FUXToolsHandPoseEditorModule::SpawnCaptureTab))
		.SetDisplayName(LOCTEXT("HandPoseCaptureTabTitle", "Hand Pose Capture"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(
		UUxtStaticHandPoseCaptureTool::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FUxtStaticHandPoseCaptureToolCustomization::MakeInstance));

	ToolMenusStartupHandle = UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUXToolsHandPoseEditorModule::RegisterMenus));
}

void FUXToolsHandPoseEditorModule::ShutdownModule()
{
	if (ToolMenusStartupHandle.IsValid())
	{
		UToolMenus::UnRegisterStartupCallback(ToolMenusStartupHandle);
		ToolMenusStartupHandle.Reset();
	}

	if (UToolMenus* ToolMenus = UToolMenus::TryGet())
	{
		ToolMenus->UnregisterOwnerByName(TEXT("UXToolsHandPoseEditor"));
	}

	if (UObjectInitialized() && FModuleManager::Get().IsModuleLoaded(TEXT("PropertyEditor")))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(UUxtStaticHandPoseCaptureTool::StaticClass()->GetFName());
	}

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(DebugTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(CaptureTabName);
}

void FUXToolsHandPoseEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(TEXT("UXToolsHandPoseEditor"));
	UToolMenu* DebugMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	FToolMenuSection& Section = DebugMenu->FindOrAddSection("Debug");
	Section.AddMenuEntry(
		"OpenHandPoseDebugPanel",
		LOCTEXT("OpenHandPoseDebugPanelLabel", "Hand Pose Debug"),
		LOCTEXT("OpenHandPoseDebugPanelTooltip", "Open the Hand Pose Debug panel."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FUXToolsHandPoseEditorModule::OpenDebugPanel)));

	Section.AddMenuEntry(
		"OpenHandPoseCapturePanel",
		LOCTEXT("OpenHandPoseCapturePanelLabel", "Hand Pose Capture"),
		LOCTEXT("OpenHandPoseCapturePanelTooltip", "Open the Hand Pose Capture panel."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FUXToolsHandPoseEditorModule::OpenCapturePanel)));
}

void FUXToolsHandPoseEditorModule::OpenDebugPanel()
{
	FGlobalTabmanager::Get()->TryInvokeTab(DebugTabName);
}

void FUXToolsHandPoseEditorModule::OpenCapturePanel()
{
	FGlobalTabmanager::Get()->TryInvokeTab(CaptureTabName);
}

TSharedRef<SDockTab> FUXToolsHandPoseEditorModule::SpawnDebugTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SUxtStaticHandPoseDebugPanel)
		];
}

TSharedRef<SDockTab> FUXToolsHandPoseEditorModule::SpawnCaptureTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SUxtStaticHandPoseCapturePanel)
		];
}

#undef LOCTEXT_NAMESPACE
     
IMPLEMENT_MODULE(FUXToolsHandPoseEditorModule, UXToolsHandPoseEditor)
