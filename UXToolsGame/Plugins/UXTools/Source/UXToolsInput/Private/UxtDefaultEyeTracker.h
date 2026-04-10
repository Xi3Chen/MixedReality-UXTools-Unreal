// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EyeTracking/IUxtEyeTracker.h"

/**
 * 
 */
class UXTOOLSINPUT_API FUxtDefaultEyeTracker :public IUxtEyeTracker
{
public:

 virtual EEyeTrackerStatus GetTrackingStatus()const override;
 virtual bool GetGazeData(FEyeTrackerGazeData& GazeData) const override;
 virtual bool GetStereoGazeData(FEyeTrackerStereoGazeData& OutGazeData) const override;
 
private:
 mutable FEyeTrackerGazeData GazeData;
 mutable FEyeTrackerStereoGazeData StereoGazeData;
 mutable EEyeTrackerStatus EyeTrackerStatus = EEyeTrackerStatus::NotConnected;
};
