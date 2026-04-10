#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

#include "UxtStaticHandPoseBindings.h"

class SMultiLineEditableTextBox;
template <typename ItemType>
class SListView;

class SUxtStaticHandPoseDebugPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUxtStaticHandPoseDebugPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	struct FDebugItem
	{
		EUxtHandPoseKeySlot Slot = EUxtHandPoseKeySlot::None;
		FText DisplayName;
		FText BindingName;
		FText PoseName;
		FUxtStaticHandPoseBindingDebugInfo DebugInfo;
		bool bHasRuntimeData = false;
	};

	void RefreshItems();
	void RefreshDetailsText();
	FReply HandleRefreshClicked();
	FReply HandleCopyClicked() const;
	TSharedRef<class ITableRow> GenerateRow(TSharedPtr<FDebugItem> Item, const TSharedRef<class STableViewBase>& OwnerTable) const;
	void HandleSelectionChanged(TSharedPtr<FDebugItem> Item, ESelectInfo::Type SelectInfo);
	FString BuildDetailsText(const FDebugItem& Item) const;
	static FString EnumToString(const UEnum* Enum, int64 Value);

	TArray<TSharedPtr<FDebugItem>> Items;
	TSharedPtr<SListView<TSharedPtr<FDebugItem>>> ListView;
	TSharedPtr<SMultiLineEditableTextBox> DetailsTextBox;
	TSharedPtr<FDebugItem> SelectedItem;
};
