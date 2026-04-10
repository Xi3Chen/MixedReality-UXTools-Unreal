#include "UxtStaticHandPoseDebugPanel.h"

#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/GenericPlatformApplicationMisc.h"
#include "HeadMountedDisplayTypes.h"
#include "UxtStaticHandPoseSettings.h"
#include "UxtStaticHandPoseSubsystem.h"
#include "UxtStaticPoseDefinition.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

void SUxtStaticHandPoseDebugPanel::Construct(const FArguments& InArgs)
{
	RefreshItems();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Configured hand pose bindings and latest runtime debug data.")))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Refresh")))
				.OnClicked(this, &SUxtStaticHandPoseDebugPanel::HandleRefreshClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Copy")))
				.OnClicked(this, &SUxtStaticHandPoseDebugPanel::HandleCopyClicked)
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(8.0f)
		[
			SNew(SSplitter)
			+ SSplitter::Slot()
			.Value(0.35f)
			[
				SAssignNew(ListView, SListView<TSharedPtr<FDebugItem>>)
				.ListItemsSource(&Items)
				.OnGenerateRow(this, &SUxtStaticHandPoseDebugPanel::GenerateRow)
				.OnSelectionChanged(this, &SUxtStaticHandPoseDebugPanel::HandleSelectionChanged)
			]
			+ SSplitter::Slot()
			.Value(0.65f)
			[
				SNew(SBorder)
				.Padding(4.0f)
				[
					SAssignNew(DetailsTextBox, SMultiLineEditableTextBox)
					.IsReadOnly(true)
					.AutoWrapText(false)
				]
			]
		]
	];

	if (Items.Num() > 0 && ListView.IsValid())
	{
		SelectedItem = Items[0];
		ListView->SetSelection(Items[0]);
	}

	RefreshDetailsText();
}

void SUxtStaticHandPoseDebugPanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Keep the panel's runtime debug snapshot aligned with the subsystem's per-frame updates.
	RefreshItems();
}

void SUxtStaticHandPoseDebugPanel::RefreshItems()
{
	const EUxtHandPoseKeySlot PreviousSelection = SelectedItem.IsValid() ? SelectedItem->Slot : EUxtHandPoseKeySlot::None;
	Items.Reset();
	SelectedItem.Reset();

	const UUxtStaticHandPoseSettings* Settings = GetDefault<UUxtStaticHandPoseSettings>();
	UUxtStaticHandPoseBindingsAsset* BindingsAsset = Settings ? Settings->BindingsAsset.LoadSynchronous() : nullptr;

	TMap<EUxtHandPoseKeySlot, FUxtStaticHandPoseBindingDebugInfo> DebugInfoBySlot;
	if (GEngine)
	{
		if (UUxtStaticHandPoseSubsystem* Subsystem = GEngine->GetEngineSubsystem<UUxtStaticHandPoseSubsystem>())
		{
			TArray<FUxtStaticHandPoseBindingDebugInfo> DebugInfos;
			Subsystem->GetLatestBindingDebugInfo(DebugInfos);
			for (const FUxtStaticHandPoseBindingDebugInfo& DebugInfo : DebugInfos)
			{
				DebugInfoBySlot.Add(DebugInfo.Slot, DebugInfo);
			}
		}
	}

	if (BindingsAsset)
	{
		TArray<EUxtHandPoseKeySlot> Slots;
		BindingsAsset->Bindings.GetKeys(Slots);
		Slots.Sort([](const EUxtHandPoseKeySlot A, const EUxtHandPoseKeySlot B)
		{
			return static_cast<uint8>(A) < static_cast<uint8>(B);
		});

		for (const EUxtHandPoseKeySlot Slot : Slots)
		{
			const FUxtStaticHandPoseBinding* Binding = BindingsAsset->GetBinding(Slot);
			if (!Binding)
			{
				continue;
			}

			TSharedPtr<FDebugItem> Item = MakeShared<FDebugItem>();
			Item->Slot = Slot;
			Item->BindingName = Binding->DebugName.IsEmpty() ? FText::FromString(TEXT("Unnamed Binding")) : Binding->DebugName;
			Item->PoseName = Binding->PoseDefinitionAsset ? FText::FromString(Binding->PoseDefinitionAsset->GetName()) : FText::FromString(TEXT("No Pose Asset"));
			Item->DisplayName = FText::Format(
				FText::FromString(TEXT("{0} - {1}")),
				FText::FromString(StaticEnum<EUxtHandPoseKeySlot>()->GetNameStringByValue(static_cast<int64>(Slot))),
				Item->BindingName);

			if (const FUxtStaticHandPoseBindingDebugInfo* DebugInfo = DebugInfoBySlot.Find(Slot))
			{
				Item->DebugInfo = *DebugInfo;
				Item->bHasRuntimeData = true;
			}

			Items.Add(Item);

			if (Slot == PreviousSelection)
			{
				SelectedItem = Item;
			}
		}
	}

	if (!SelectedItem.IsValid() && Items.Num() > 0)
	{
		SelectedItem = Items[0];
	}

	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
		if (SelectedItem.IsValid())
		{
			ListView->SetSelection(SelectedItem);
		}
	}

	RefreshDetailsText();
}

void SUxtStaticHandPoseDebugPanel::RefreshDetailsText()
{
	if (!DetailsTextBox.IsValid())
	{
		return;
	}

	if (!SelectedItem.IsValid())
	{
		DetailsTextBox->SetText(FText::FromString(TEXT("No hand pose binding selected.")));
		return;
	}

	DetailsTextBox->SetText(FText::FromString(BuildDetailsText(*SelectedItem)));
}

FReply SUxtStaticHandPoseDebugPanel::HandleRefreshClicked()
{
	RefreshItems();
	return FReply::Handled();
}

FReply SUxtStaticHandPoseDebugPanel::HandleCopyClicked() const
{
	
	const FString TextToCopy = SelectedItem.IsValid() ? BuildDetailsText(*SelectedItem) : FString(TEXT("No hand pose binding selected."));
	FGenericPlatformApplicationMisc::ClipboardCopy(*TextToCopy);
	return FReply::Handled();
}

TSharedRef<ITableRow> SUxtStaticHandPoseDebugPanel::GenerateRow(
	TSharedPtr<FDebugItem> Item,
	const TSharedRef<STableViewBase>& OwnerTable) const
{
	return SNew(STableRow<TSharedPtr<FDebugItem>>, OwnerTable)
	[
		SNew(STextBlock)
		.Text(Item.IsValid() ? Item->DisplayName : FText::FromString(TEXT("Invalid Item")))
	];
}

void SUxtStaticHandPoseDebugPanel::HandleSelectionChanged(TSharedPtr<FDebugItem> Item, ESelectInfo::Type SelectInfo)
{
	SelectedItem = Item;
	RefreshDetailsText();
}

FString SUxtStaticHandPoseDebugPanel::BuildDetailsText(const FDebugItem& Item) const
{
	const UEnum* SlotEnum = StaticEnum<EUxtHandPoseKeySlot>();
	const UEnum* HandEnum = StaticEnum<EControllerHand>();
	const UEnum* StateEnum = StaticEnum<EUxtStaticHandPoseBindingState>();
	const UEnum* FailureEnum = StaticEnum<EUxtStaticPoseFailureReason>();
	const UEnum* JointEnum = StaticEnum<EHandKeypoint>();

	FString Result;
	Result += FString::Printf(TEXT("Slot: %s\n"), *EnumToString(SlotEnum, static_cast<int64>(Item.Slot)));
	Result += FString::Printf(TEXT("Binding Name: %s\n"), *Item.BindingName.ToString());
	Result += FString::Printf(TEXT("Pose Asset: %s\n"), *Item.PoseName.ToString());

	if (!Item.bHasRuntimeData)
	{
		Result += TEXT("\nRuntime debug data is unavailable. Start PIE or play in editor to populate live state.\n");
		return Result;
	}

	Result += TEXT("\nLatest Runtime Debug\n");
	Result += FString::Printf(TEXT("Key: %s\n"), *Item.DebugInfo.Key.GetDisplayName().ToString());
	Result += FString::Printf(TEXT("Triggering Hand: %s\n"), *EnumToString(HandEnum, static_cast<int64>(Item.DebugInfo.TriggeringHand.GetValue())));
	Result += FString::Printf(TEXT("Score: %.4f\n"), Item.DebugInfo.Score);
	Result += FString::Printf(TEXT("Passed: %s\n"), Item.DebugInfo.bPassed ? TEXT("True") : TEXT("False"));
	Result += FString::Printf(TEXT("State: %s\n"), *EnumToString(StateEnum, static_cast<int64>(Item.DebugInfo.State)));
	Result += FString::Printf(TEXT("Failure Reason: %s\n"), *EnumToString(FailureEnum, static_cast<int64>(Item.DebugInfo.Evaluation.FailureReason)));
	Result += FString::Printf(TEXT("Primary Debug Value: %.4f\n"), Item.DebugInfo.Evaluation.PrimaryDebugValue);
	Result += FString::Printf(TEXT("Debug Joint: %s\n"), *EnumToString(JointEnum, static_cast<int64>(Item.DebugInfo.Evaluation.DebugJoint.GetValue())));

	return Result;
}

FString SUxtStaticHandPoseDebugPanel::EnumToString(const UEnum* Enum, int64 Value)
{
	return Enum ? Enum->GetNameStringByValue(Value) : FString(TEXT("Unknown"));
}
