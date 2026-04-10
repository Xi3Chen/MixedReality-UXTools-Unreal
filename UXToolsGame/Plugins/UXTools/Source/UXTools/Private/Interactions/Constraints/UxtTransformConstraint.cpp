// Copyright (c) 2020 Microsoft Corporation.
// Licensed under the MIT License.

#include "Interactions/Constraints/UxtTransformConstraint.h"

#include "GameFramework/Actor.h"
#include "Utils/UxtFunctionLibrary.h"

UUxtTransformConstraint::UUxtTransformConstraint()
{
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_LastDemotable;
}

void UUxtTransformConstraint::Initialize(const FTransform& WorldPose)
{
	WorldPoseOnManipulationStart = WorldPose;
}
