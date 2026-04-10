#include "UxtStaticHandPoseCaptureToolCustomization.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "UxtStaticHandPoseCaptureTool.h"

TSharedRef<IDetailCustomization> FUxtStaticHandPoseCaptureToolCustomization::MakeInstance()
{
	return MakeShared<FUxtStaticHandPoseCaptureToolCustomization>();
}

void FUxtStaticHandPoseCaptureToolCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	CaptureTool.Reset();

	for (const TWeakObjectPtr<UObject>& Object : Objects)
	{
		if (UUxtStaticHandPoseCaptureTool* Tool = Cast<UUxtStaticHandPoseCaptureTool>(Object.Get()))
		{
			CaptureTool = Tool;
			break;
		}
	}

	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(TEXT("Capture"));
	Category.AddCustomRow(FText::FromString(TEXT("Capture Actions")))
	.WholeRowContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Capture Left")))
			.OnClicked(this, &FUxtStaticHandPoseCaptureToolCustomization::HandleCaptureLeftClicked)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Capture Right")))
			.OnClicked(this, &FUxtStaticHandPoseCaptureToolCustomization::HandleCaptureRightClicked)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Capture Both")))
			.OnClicked(this, &FUxtStaticHandPoseCaptureToolCustomization::HandleCaptureBothClicked)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Save Asset")))
			.OnClicked(this, &FUxtStaticHandPoseCaptureToolCustomization::HandleSaveClicked)
		]
	];
}

FReply FUxtStaticHandPoseCaptureToolCustomization::HandleCaptureLeftClicked() const
{
	if (UUxtStaticHandPoseCaptureTool* Tool = CaptureTool.Get())
	{
		Tool->StartCapture(EUxtStaticHandPoseCaptureMode::Left, FPlatformTime::Seconds());
	}

	return FReply::Handled();
}

FReply FUxtStaticHandPoseCaptureToolCustomization::HandleCaptureRightClicked() const
{
	if (UUxtStaticHandPoseCaptureTool* Tool = CaptureTool.Get())
	{
		Tool->StartCapture(EUxtStaticHandPoseCaptureMode::Right, FPlatformTime::Seconds());
	}

	return FReply::Handled();
}

FReply FUxtStaticHandPoseCaptureToolCustomization::HandleCaptureBothClicked() const
{
	if (UUxtStaticHandPoseCaptureTool* Tool = CaptureTool.Get())
	{
		Tool->StartCapture(EUxtStaticHandPoseCaptureMode::Both, FPlatformTime::Seconds());
	}

	return FReply::Handled();
}

FReply FUxtStaticHandPoseCaptureToolCustomization::HandleSaveClicked() const
{
	if (UUxtStaticHandPoseCaptureTool* Tool = CaptureTool.Get())
	{
		Tool->SaveTargetAsset();
	}

	return FReply::Handled();
}
