// Fill out your copyright notice in the Description page of Project Settings.


#include "EyeTracking/IUxtEyeTracker.h"

#include "HeadMountedDisplayTypes.h"
#include "UxtTrackingControllerSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

class FHMDTracker :public IUxtEyeTracker
{
private:
	bool TryGetCameraLocationAndForwardVector(FVector& CameraLocation,FVector&CameraForwardVector) const
	{
		if(UUxtTrackingControllerSubsystem* UxtTrackingControllerSubsystem = GEngine->GetEngineSubsystem<UUxtTrackingControllerSubsystem>())
		{
			UCameraComponent* CameraComponent = UxtTrackingControllerSubsystem->GetPlayerFollowCameraComponent();
			if(CameraComponent == nullptr)
			{
				return false;
			}
			if(CameraComponent->GetOwner() == UGameplayStatics::GetPlayerController(CameraComponent,0) ->GetViewTarget())
			{
				CameraLocation = CameraComponent->GetComponentLocation();
				CameraForwardVector = CameraComponent->GetForwardVector();
				return true;
			}
			else if(APlayerCameraManager* CameraManager =  UGameplayStatics::GetPlayerCameraManager(CameraComponent,0))
			{
				CameraLocation = CameraManager->GetCameraLocation();
				CameraForwardVector =  CameraManager->GetCameraRotation().Vector();
				return true;
			}
			return false;
		}
		return false;
	}
public:
	virtual EEyeTrackerStatus GetTrackingStatus()const override
	{
		return EEyeTrackerStatus::Tracking;
	}

	virtual bool GetGazeData(FEyeTrackerGazeData& GazeData) const override
	{
		FVector GazeOrigin, GazeDirection;
		if (TryGetCameraLocationAndForwardVector(GazeOrigin, GazeDirection))
		{
			GazeData.ConfidenceValue = 1.0;
			GazeData.GazeOrigin = GazeOrigin;
			GazeData.GazeDirection = GazeDirection;
			GazeData.FixationPoint = GazeOrigin + GazeDirection * 100;
			GazeData.LeftPupilDiameter = 1.0;
			GazeData.RightPupilDiameter = 1.0;
			GazeData.bIsLeftEyeBlink = false;
			GazeData.bIsLeftEyeBlink = false;
			return true;
		}
		return false;
	}
	
	virtual bool GetStereoGazeData(FEyeTrackerStereoGazeData& OutGazeData) const override
	{
		FVector GazeOrigin, GazeDirection;
		if (TryGetCameraLocationAndForwardVector(GazeOrigin, GazeDirection))
		{
			OutGazeData.ConfidenceValue = 1.0;
			OutGazeData.FixationPoint = GazeOrigin + GazeDirection * 100;
			OutGazeData.LeftEyeDirection = GazeDirection;
			OutGazeData.LeftEyeOrigin = GazeOrigin;
			OutGazeData.RightEyeDirection = GazeDirection;
			OutGazeData.RightEyeOrigin = GazeOrigin;
			return true;
		}
		return false;
	}

};

FName IUxtEyeTracker::GetModularFeatureName()
{
	static FName FeatureName = FName(TEXT("UxtEyeTracker"));
	return FeatureName;
}

IUxtEyeTracker& IUxtEyeTracker::Get()
{
	

	IModularFeatures& Features = IModularFeatures::Get();
	FName FeatureName = GetModularFeatureName();

	if (Features.IsModularFeatureAvailable(FeatureName))
	{
		return Features.GetModularFeature<IUxtEyeTracker>(FeatureName);
	}
	return GetHdmTracker();
}

IUxtEyeTracker& IUxtEyeTracker::GetHdmTracker()
{
	static FHMDTracker HMDTracker;
	return HMDTracker;
}
