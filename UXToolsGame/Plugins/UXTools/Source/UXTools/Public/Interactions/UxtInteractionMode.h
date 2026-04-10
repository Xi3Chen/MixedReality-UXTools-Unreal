// Copyright (c) 2020 Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

#include "UxtInteractionMode.generated.h"

/** Interaction modes supported */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EUxtInteractionMode : uint8
{
	None = 0 UMETA(Hidden),
	/** Interact with poke and grab targets (see IUxtPokeTarget and IUxtGrabTarget) */
	Near = 1 << 0,
	/** Interact with far targets (see IUxtFarTarget) */
	Far = 1 << 1,
	/** Interact with head orientation targets (see IUxtHeadOrientationTarget).
	  * NOTE: HeadOrientation cannot be used together with Near or Far modes.       */
	HeadOrientation = 1 << 2,
	/** When using head-orientation/eye-based interactions, force the gaze ray to use HMD fallback tracking. */
	ForceHmdTracker = 1 << 3,
};
ENUM_CLASS_FLAGS(EUxtInteractionMode)

/** Grab modes supported */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EUxtGrabMode : uint8
{
	None = 0 UMETA(Hidden),
	/** Grab objects with one hand */
	OneHanded = 1 << 0,
	/** Grab objects with two hands */
	TwoHanded = 1 << 1,
};
ENUM_CLASS_FLAGS(EUxtGrabMode)
