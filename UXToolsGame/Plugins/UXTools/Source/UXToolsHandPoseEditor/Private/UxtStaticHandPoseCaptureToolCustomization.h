#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class UUxtStaticHandPoseCaptureTool;

class FUxtStaticHandPoseCaptureToolCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	FReply HandleCaptureLeftClicked() const;
	FReply HandleCaptureRightClicked() const;
	FReply HandleCaptureBothClicked() const;
	FReply HandleSaveClicked() const;

	TWeakObjectPtr<UUxtStaticHandPoseCaptureTool> CaptureTool;
};
