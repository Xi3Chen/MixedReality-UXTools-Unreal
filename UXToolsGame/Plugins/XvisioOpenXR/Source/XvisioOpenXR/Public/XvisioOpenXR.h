/*
 * @author: eddy
 * @Date: 2023-04-28 13:44:17
 * @LastEditTime: 2023-05-25 17:05:37
 * @LastEditors: eddy
 * @brief:
 * @copyright: Xvisio Tec
 * @FilePath: \XvisioOpenXR\Public\XvisioOpenXR.h
 * 唵嘛呢叭咪吽
 */
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IXvisioOpenXRPlugin.h"
// #include "OpenXRHandTracking.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XvisioOpenXR.generated.h"

UCLASS(ClassGroup = OpenXR)
class XVISIOOPENXR_API UXvisioOpenXRFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	Turn Hand Mesh

	@param On true if enable
	@return true if the command successes
	*/
	// UFUNCTION(BlueprintCallable, Category = "XvisioOpenXR|OpenXR")
	// static bool SetUseHandMesh(EHandMeshStatus Mode);
};