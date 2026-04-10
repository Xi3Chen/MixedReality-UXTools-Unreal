#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class SDockTab;

class FUXToolsHandPoseEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
	void RegisterMenus();
	void OpenDebugPanel();
	void OpenCapturePanel();
	TSharedRef<SDockTab> SpawnDebugTab(const class FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> SpawnCaptureTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	static const FName DebugTabName;
	static const FName CaptureTabName;
	FDelegateHandle ToolMenusStartupHandle;
};
