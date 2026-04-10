#include "UxtStaticHandPoseCaptureTool.h"

#include "Engine/Engine.h"
#include "HAL/PlatformTime.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UxtStaticHandPoseSubsystem.h"
#include "UxtStaticPoseEvaluator.h"
#include "UxtStaticPoseDefinition.h"

#include "UObject/SavePackage.h"

UUxtStaticHandPoseCaptureTool::UUxtStaticHandPoseCaptureTool()
{
	StatusText = FText::FromString(TEXT("Select a pose asset, choose a delay, then capture left, right, or both hands."));
	PendingCaptureText = FText::FromString(TEXT("No capture scheduled."));
}

void UUxtStaticHandPoseCaptureTool::StartCapture(EUxtStaticHandPoseCaptureMode InCaptureMode, double CurrentTimeSeconds)
{
	UpdateTrackingStatus();

	if (!TargetPoseAsset)
	{
		SetStatus(TEXT("Select a target pose asset before starting capture."));
		return;
	}

	PendingCaptureMode = InCaptureMode;
	CaptureExecuteTimeSeconds = CurrentTimeSeconds + FMath::Max(0.0f, DelaySeconds);
	CountdownSeconds = static_cast<float>(FMath::Max(0.0, CaptureExecuteTimeSeconds - CurrentTimeSeconds));
	PendingCaptureText = FText::FromString(CaptureModeToString(InCaptureMode));
	SetStatus(FString::Printf(TEXT("%s capture scheduled. Hold the pose. Recording in %.2f seconds."), *CaptureModeToString(InCaptureMode), FMath::Max(0.0, CaptureExecuteTimeSeconds - CurrentTimeSeconds)));
	Modify();
	TargetPoseAsset->Modify();
}

bool UUxtStaticHandPoseCaptureTool::Tick(double CurrentTimeSeconds)
{
	UpdateTrackingStatus();

	if (PendingCaptureMode == EUxtStaticHandPoseCaptureMode::None)
	{
		CountdownSeconds = 0.0f;
		PendingCaptureText = FText::FromString(TEXT("No capture scheduled."));
		return false;
	}

	if (CurrentTimeSeconds >= CaptureExecuteTimeSeconds)
	{
		return ExecuteCapture();
	}

	CountdownSeconds = static_cast<float>(FMath::Max(0.0, CaptureExecuteTimeSeconds - CurrentTimeSeconds));
	PendingCaptureText = FText::FromString(CaptureModeToString(PendingCaptureMode));
	SetStatus(FString::Printf(TEXT("%s capture scheduled. Hold the pose. Recording in %.2f seconds. Tracking state now: Left=%s Right=%s."), *CaptureModeToString(PendingCaptureMode), CountdownSeconds, bLeftHandTracked ? TEXT("tracked") : TEXT("not tracked"), bRightHandTracked ? TEXT("tracked") : TEXT("not tracked")));
	return true;
}

bool UUxtStaticHandPoseCaptureTool::SaveTargetAsset()
{
	if (!TargetPoseAsset)
	{
		SetStatus(TEXT("Select a target pose asset before saving."));
		return true;
	}

	UPackage* Package = TargetPoseAsset->GetOutermost();
	if (!Package)
	{
		SetStatus(TEXT("Unable to resolve the target asset package."));
		return true;
	}

	const FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
	if (PackageFileName.IsEmpty())
	{
		SetStatus(TEXT("Unable to build a package filename for the target asset."));
		return true;
	}

	Package->MarkPackageDirty();

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;
	const bool bSaved = UPackage::SavePackage(Package, TargetPoseAsset, *PackageFileName, SaveArgs);
	SetStatus(bSaved ? FString::Printf(TEXT("Saved pose asset: %s"), *TargetPoseAsset->GetName()) : FString::Printf(TEXT("Failed to save pose asset: %s"), *TargetPoseAsset->GetName()));
	return true;
}

bool UUxtStaticHandPoseCaptureTool::ConsumeRefreshRequest()
{
	const bool bShouldRefresh = bNeedsRefresh;
	bNeedsRefresh = false;
	return bShouldRefresh;
}

bool UUxtStaticHandPoseCaptureTool::ExecuteCapture()
{
	UpdateTrackingStatus();
	const EUxtStaticHandPoseCaptureMode CaptureMode = PendingCaptureMode;
	PendingCaptureMode = EUxtStaticHandPoseCaptureMode::None;
	CountdownSeconds = 0.0f;
	PendingCaptureText = FText::FromString(TEXT("No capture scheduled."));

	if (!TargetPoseAsset)
	{
		SetStatus(TEXT("Select a target pose asset before starting capture."));
		return true;
	}

	TargetPoseAsset->Modify();

	bool bLeftCaptured = false;
	bool bRightCaptured = false;

	if (CaptureMode == EUxtStaticHandPoseCaptureMode::Left || CaptureMode == EUxtStaticHandPoseCaptureMode::Both)
	{
		bLeftCaptured = CaptureHand(EControllerHand::Left);
	}

	if (CaptureMode == EUxtStaticHandPoseCaptureMode::Right || CaptureMode == EUxtStaticHandPoseCaptureMode::Both)
	{
		bRightCaptured = CaptureHand(EControllerHand::Right);
	}

	if (CaptureMode == EUxtStaticHandPoseCaptureMode::Left)
	{
		SetStatus(bLeftCaptured
			? TEXT("Captured left hand pose data. Save the asset to persist changes.")
			: FString::Printf(TEXT("Left hand capture failed. Tracking at capture time: Left=%s Right=%s."), bLeftHandTracked ? TEXT("tracked") : TEXT("not tracked"), bRightHandTracked ? TEXT("tracked") : TEXT("not tracked")));
	}
	else if (CaptureMode == EUxtStaticHandPoseCaptureMode::Right)
	{
		SetStatus(bRightCaptured
			? TEXT("Captured right hand pose data. Save the asset to persist changes.")
			: FString::Printf(TEXT("Right hand capture failed. Tracking at capture time: Left=%s Right=%s."), bLeftHandTracked ? TEXT("tracked") : TEXT("not tracked"), bRightHandTracked ? TEXT("tracked") : TEXT("not tracked")));
	}
	else
	{
		if (bLeftCaptured || bRightCaptured)
		{
			SetStatus(FString::Printf(TEXT("Both-hand capture finished. Left: %s. Right: %s. Tracking at capture time: Left=%s Right=%s. Save the asset to persist changes."), bLeftCaptured ? TEXT("ok") : TEXT("failed"), bRightCaptured ? TEXT("ok") : TEXT("failed"), bLeftHandTracked ? TEXT("tracked") : TEXT("not tracked"), bRightHandTracked ? TEXT("tracked") : TEXT("not tracked")));
		}
		else
		{
			SetStatus(FString::Printf(TEXT("Both-hand capture failed. No pose data was recorded. Tracking at capture time: Left=%s Right=%s."), bLeftHandTracked ? TEXT("tracked") : TEXT("not tracked"), bRightHandTracked ? TEXT("tracked") : TEXT("not tracked")));
		}
	}

	if (bLeftCaptured || bRightCaptured)
	{
		TargetPoseAsset->MarkPackageDirty();
	}

	return true;
}

bool UUxtStaticHandPoseCaptureTool::CaptureHand(EControllerHand Hand)
{
	if (!GEngine)
	{
		return false;
	}

	if (UUxtStaticHandPoseSubsystem* Subsystem = GEngine->GetEngineSubsystem<UUxtStaticHandPoseSubsystem>())
	{
		return Subsystem->CapturePoseDefinition(TargetPoseAsset, Hand);
	}

	return false;
}

void UUxtStaticHandPoseCaptureTool::UpdateTrackingStatus()
{
	bLeftHandTracked = false;
	bRightHandTracked = false;

	if (!GEngine)
	{
		return;
	}

	if (UUxtStaticHandPoseSubsystem* Subsystem = GEngine->GetEngineSubsystem<UUxtStaticHandPoseSubsystem>())
	{
		FUxtTrackedHandFrame LeftHandFrame;
		FUxtTrackedHandFrame RightHandFrame;
		bLeftHandTracked = Subsystem->GetTrackedHandFrame(EControllerHand::Left, LeftHandFrame);
		bRightHandTracked = Subsystem->GetTrackedHandFrame(EControllerHand::Right, RightHandFrame);
	}
}

void UUxtStaticHandPoseCaptureTool::SetStatus(const FString& Message)
{
	StatusText = FText::FromString(Message);
	bNeedsRefresh = true;
}

FString UUxtStaticHandPoseCaptureTool::CaptureModeToString(EUxtStaticHandPoseCaptureMode InCaptureMode)
{
	switch (InCaptureMode)
	{
	case EUxtStaticHandPoseCaptureMode::Left:
		return TEXT("Left hand");
	case EUxtStaticHandPoseCaptureMode::Right:
		return TEXT("Right hand");
	case EUxtStaticHandPoseCaptureMode::Both:
		return TEXT("Both hands");
	default:
		return TEXT("Unknown");
	}
}
