// Copyright (c) 2020 Microsoft Corporation.
// Licensed under the MIT License.

#include "Controls/UxtToggleStateComponent.h"

UUxtToggleStateComponent::UUxtToggleStateComponent()
{
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_LastDemotable;
}

void UUxtToggleStateComponent::SetIsChecked(bool IsChecked)
{
	if (bIsChecked != IsChecked)
	{
		bIsChecked = IsChecked;
		OnToggled.Broadcast(this);
	}
}
