/*
 * @author: eddy
 * @Date: 2023-05-16 14:14:49
 * @LastEditTime: 2023-05-22 14:44:13
 * @LastEditors: eddy
 * @brief:
 * @copyright: Xvisio Tec
 * @FilePath: \XvisioOpenXR\Public\IXvisioOpenXRPlugin.h
 * 唵嘛呢叭咪吽
 */
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "IOpenXRExtensionPlugin.h"
#include "Modules/ModuleInterface.h"
#include "HeadMountedDisplayBase.h"
//UENUM(BlueprintType, Category = "MicrosoftOpenXR|OpenXR")
//enum class EHandMeshStatus : uint8
//{
//    NotInitialised = 0 UMETA(Hidden),
//    Disabled = 1,
//    EnabledTrackingGeometry = 2,
//    EnabledXRVisualization = 3
//};

class IXvisioOpenXRPlugin : public IOpenXRExtensionPlugin, public IModuleInterface
{
};