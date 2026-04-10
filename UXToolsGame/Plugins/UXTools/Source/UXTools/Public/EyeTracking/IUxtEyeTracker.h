// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EyeTrackerTypes.h"


/**
 * 
 */
class UXTOOLS_API IUxtEyeTracker:public IModularFeature
{
public:
	static FName GetModularFeatureName();
	/** Returns the currently registered eye tracker or nullptr if none */
	static IUxtEyeTracker& Get();
	static  IUxtEyeTracker&GetHdmTracker();
	virtual ~IUxtEyeTracker(){}

	
	virtual EEyeTrackerStatus GetTrackingStatus()const =0;
	virtual bool GetGazeData(FEyeTrackerGazeData& GazeData) const =0;
	virtual bool GetStereoGazeData(FEyeTrackerStereoGazeData& OutGazeData) const =0;
	
};
