#include "UxtStaticHandPoseCapturePanel.h"

#include "PropertyEditorModule.h"
#include "UxtStaticHandPoseCaptureTool.h"

void SUxtStaticHandPoseCapturePanel::Construct(const FArguments& InArgs)
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;

	CaptureTool = TStrongObjectPtr<UUxtStaticHandPoseCaptureTool>(NewObject<UUxtStaticHandPoseCaptureTool>(GetTransientPackage()));
	DetailsView = PropertyModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(CaptureTool.Get());

	ChildSlot
	[
		DetailsView.ToSharedRef()
	];
}

void SUxtStaticHandPoseCapturePanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (CaptureTool.IsValid())
	{
		CaptureTool->Tick(InCurrentTime);

		if (CaptureTool->ConsumeRefreshRequest() && DetailsView.IsValid())
		{
			DetailsView->ForceRefresh();
		}
	}
}
