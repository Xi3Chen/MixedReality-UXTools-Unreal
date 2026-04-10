#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class IDetailsView;
class UUxtStaticHandPoseCaptureTool;

class SUxtStaticHandPoseCapturePanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUxtStaticHandPoseCapturePanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	TSharedPtr<IDetailsView> DetailsView;
	TStrongObjectPtr<UUxtStaticHandPoseCaptureTool> CaptureTool;
};
