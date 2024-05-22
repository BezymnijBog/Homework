// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryViewWidget.h"

#include "InventorySlotWidget.h"

#include "Components/GridPanel.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"


void UInventoryViewWidget::InitializeViewWidget(TArray<FInventorySlot>& InventorySlots)
{
	for (FInventorySlot& Item : InventorySlots)
	{
		AddItemSlotView(Item);
	}
}

void UInventoryViewWidget::AddItemSlotView(FInventorySlot& SlotToAdd)
{
	check(InventorySlotWidgetClass.Get());

	UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(this, InventorySlotWidgetClass.Get());

	if (SlotWidget)
	{
		SlotWidget->InitializeItemSlot(SlotToAdd);

		const int32 CurrentSlotCount = GridPanelItemSlots->GetChildrenCount();
		const int32 CurrentSlotRow = CurrentSlotCount / ColumnCount;
		const int32 CurrentSlotColumn = CurrentSlotCount % ColumnCount;
		GridPanelItemSlots->AddChildToGrid(SlotWidget, CurrentSlotRow, CurrentSlotColumn);

		SlotWidget->UpdateView();
	}
}
