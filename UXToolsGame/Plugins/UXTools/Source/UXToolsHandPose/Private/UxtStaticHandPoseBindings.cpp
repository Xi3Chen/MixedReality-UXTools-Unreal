#include "UxtStaticHandPoseBindings.h"

const FUxtStaticHandPoseBinding* UUxtStaticHandPoseBindingsAsset::GetBinding(EUxtHandPoseKeySlot Slot) const
{
	return Bindings.Find(Slot);
}

bool UUxtStaticHandPoseBindingsAsset::IsValidSlot(EUxtHandPoseKeySlot Slot) const
{
	return Slot != EUxtHandPoseKeySlot::None && Bindings.Contains(Slot);
}

int32 UUxtStaticHandPoseBindingsAsset::GetBindingCount() const
{
	return Bindings.Num();
}
