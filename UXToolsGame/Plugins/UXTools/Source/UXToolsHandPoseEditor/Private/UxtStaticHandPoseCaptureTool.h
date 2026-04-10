#pragma once

#include "CoreMinimal.h"
#include "HeadMountedDisplayTypes.h"
#include "UObject/Object.h"

#include "UxtStaticHandPoseCaptureTool.generated.h"

class UUxtStaticPoseDefinition;

UENUM()
enum class EUxtStaticHandPoseCaptureMode : uint8
{
	None,
	Left,
	Right,
	Both,
};

UCLASS()
class UXTOOLSHANDPOSEEDITOR_API UUxtStaticHandPoseCaptureTool : public UObject
{
	GENERATED_BODY()

public:
	UUxtStaticHandPoseCaptureTool();

	void StartCapture(EUxtStaticHandPoseCaptureMode InCaptureMode, double CurrentTimeSeconds);
	bool Tick(double CurrentTimeSeconds);
	bool SaveTargetAsset();
	bool ConsumeRefreshRequest();

	UPROPERTY(EditAnywhere, Category = "Capture")
	TObjectPtr<UUxtStaticPoseDefinition> TargetPoseAsset = nullptr;

	UPROPERTY(EditAnywhere, Category = "Capture", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DelaySeconds = 3.0f;

	UPROPERTY(VisibleAnywhere, Category = "Capture")
	FText StatusText;

	UPROPERTY(VisibleAnywhere, Category = "Capture")
	bool bLeftHandTracked = false;

	UPROPERTY(VisibleAnywhere, Category = "Capture")
	bool bRightHandTracked = false;

	UPROPERTY(VisibleAnywhere, Category = "Capture")
	FText PendingCaptureText;

	UPROPERTY(VisibleAnywhere, Category = "Capture")
	float CountdownSeconds = 0.0f;

private:
	bool ExecuteCapture();
	bool CaptureHand(EControllerHand Hand);
	void UpdateTrackingStatus();
	void SetStatus(const FString& Message);
	static FString CaptureModeToString(EUxtStaticHandPoseCaptureMode InCaptureMode);

	EUxtStaticHandPoseCaptureMode PendingCaptureMode = EUxtStaticHandPoseCaptureMode::None;
	double CaptureExecuteTimeSeconds = 0.0;
	bool bNeedsRefresh = false;
};
