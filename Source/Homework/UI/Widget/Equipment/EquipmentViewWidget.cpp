// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentViewWidget.h"

#include "EquipmentSlotWidget.h"
#include "Actors/Equipment/EquipableItem.h"
#include "Components/VerticalBox.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void UEquipmentViewWidget::InitializeEquipmentWidget(UCharacterEquipmentComponent* EquipmentComponent)
{
	LinkedEquipmentComponent = EquipmentComponent;
	const TArray<AEquipableItem*>& Items = LinkedEquipmentComponent->GetItems();

	for (int32 Index = 1; Index < Items.Num(); ++Index)
	{
		AddEquipmentSlotView(Items[Index], Index);
	}
}

void UEquipmentViewWidget::AddEquipmentSlotView(AEquipableItem* LinkToEquipment, int32 SlotIndex)
{
	check(IsValid(DefaultSlotViewClass.Get()));

	UEquipmentSlotWidget* SlotWidget = CreateWidget<UEquipmentSlotWidget>(this, DefaultSlotViewClass.Get());

	if (SlotWidget)
	{
		SlotWidget->InitializeEquipmentSlot(LinkToEquipment, SlotIndex);

		VBEquipmentSlots->AddChildToVerticalBox(SlotWidget);
		SlotWidget->UpdateView();
		SlotWidget->OnEquipmentDropInSlot.BindUObject(this, &UEquipmentViewWidget::EquipEquipmentToSlot);
		SlotWidget->OnEquipmentRemoveFromSlot.BindUObject(this, &UEquipmentViewWidget::RemoveEquipmentFromSlot);
	}
}

void UEquipmentViewWidget::UpdateSlots(int32 SlotIndex)
{
	UEquipmentSlotWidget* WidgetToUpdate = Cast<UEquipmentSlotWidget>(VBEquipmentSlots->GetChildAt(SlotIndex - 1));
	if (IsValid(WidgetToUpdate))
	{
		WidgetToUpdate->InitializeEquipmentSlot(LinkedEquipmentComponent->GetItems()[SlotIndex], SlotIndex);
		WidgetToUpdate->UpdateView();
	}
}

bool UEquipmentViewWidget::EquipEquipmentToSlot(const TSubclassOf<AEquipableItem>& EquipmentClass, int32 SenderIndex)
{
	if (LinkedEquipmentComponent->AddEquipmentItemToSlot(EquipmentClass, SenderIndex))
	{
		UpdateSlots(SenderIndex);
		return true;
	}
	return false;
}

void UEquipmentViewWidget::RemoveEquipmentFromSlot(int32 SlotIndex)
{
	LinkedEquipmentComponent->RemoveItemFromSlot(SlotIndex);
}
