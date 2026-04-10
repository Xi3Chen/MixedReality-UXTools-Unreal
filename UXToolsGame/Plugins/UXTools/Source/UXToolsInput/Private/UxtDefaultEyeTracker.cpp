// Fill out your copyright notice in the Description page of Project Settings.


#include "UxtDefaultEyeTracker.h"

#include "EyeTrackerFunctionLibrary.h"

EEyeTrackerStatus FUxtDefaultEyeTracker::GetTrackingStatus() const
{
	IEyeTracker const* const ET = GEngine ? GEngine->EyeTrackingDevice.Get() : nullptr;
	if (ET)
	{
		EyeTrackerStatus = ET->GetEyeTrackerStatus();
		return EyeTrackerStatus;
	}
	return EyeTrackerStatus;
}

bool FUxtDefaultEyeTracker::GetGazeData(FEyeTrackerGazeData& OutGazeData) const
{
	const bool bHasGazeData = UEyeTrackerFunctionLibrary::GetGazeData(OutGazeData);
	GazeData = OutGazeData;
	EyeTrackerStatus = GetTrackingStatus();
	return bHasGazeData;
}

bool FUxtDefaultEyeTracker::GetStereoGazeData(FEyeTrackerStereoGazeData& OutGazeData) const
{
	const bool bHasStereoGazeData = UEyeTrackerFunctionLibrary::GetStereoGazeData(OutGazeData);
	StereoGazeData = OutGazeData;
	EyeTrackerStatus = GetTrackingStatus();
	return bHasStereoGazeData;
}
