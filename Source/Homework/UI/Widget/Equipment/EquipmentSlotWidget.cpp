// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentSlotWidget.h"

#include "Actors/Equipment/EquipableItem.h"
#include "Actors/Interactive/Pickables/PickableItem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Inventory/Items/InventoryItem.h"
#include "Inventory/Items/Equipables/WeaponInventoryItem.h"
#include "UI/Widget/Inventory/InventorySlotWidget.h"
#include "Utils/HWDataTableUtils.h"

void UEquipmentSlotWidget::InitializeEquipmentSlot(TWeakObjectPtr<AEquipableItem> Equipment, int32 Index)
{
	if (!Equipment.IsValid())
	{
		return;
	}

	LinkedEquipableItem = Equipment;
	SlotIndexComponent = Index;

	FWeaponTableRow* EquipmentData = HWDataTableUtils::FindWeaponData(Equipment->GetDataTableID());
	if (EquipmentData)
	{
		LinkedInventoryItem = NewObject<UWeaponInventoryItem>(Equipment->GetOwner());
		LinkedInventoryItem->Initialize(Equipment->GetDataTableID(), EquipmentData->WeaponItemDescription);
		LinkedInventoryItem->SetEquipWeaponClass(EquipmentData->EquipableActorClass);
	}
}

void UEquipmentSlotWidget::UpdateView()
{
	UTexture2D* Texture = LinkedEquipableItem.IsValid() ? LinkedInventoryItem->GetDescription().Icon : nullptr;
	FText WeaponName = LinkedEquipableItem.IsValid() ? LinkedInventoryItem->GetDescription().Name : FText::FromName(NAME_None);
	ImageWeaponItem->SetBrushFromTexture(Texture);
	TBWeaponName->SetText(WeaponName);
}

FReply UEquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!LinkedEquipableItem.IsValid())
	{
		return FReply::Handled();
	}

	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}

void UEquipmentSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	check(DragAndDropWidgetClass.Get());

	if (!LinkedInventoryItem.IsValid())
	{
		return;
	}

	UDragDropOperation* DragOperation = Cast<UDragDropOperation>(UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass()));

	UInventorySlotWidget* DragWidget = CreateWidget<UInventorySlotWidget>(GetOwningPlayer(), DragAndDropWidgetClass.Get());
	DragWidget->SetItemIcon(LinkedInventoryItem->GetDescription().Icon);

	DragOperation->DefaultDragVisual = DragWidget;
	DragOperation->Pivot = EDragPivot::CenterCenter;
	DragOperation->Payload = LinkedInventoryItem.Get();
	OutOperation = DragOperation;

	LinkedEquipableItem.Reset();
	bool bIsExecuted = OnEquipmentRemoveFromSlot.ExecuteIfBound(SlotIndexComponent);

	UpdateView();
}

bool UEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const UWeaponInventoryItem* OperationObject = Cast<UWeaponInventoryItem>(InOperation->Payload);
	if (IsValid(OperationObject))
	{
		return OnEquipmentDropInSlot.Execute(OperationObject->GetEquipWeaponClass(), SlotIndexComponent);
	}
	return false;
}

void UEquipmentSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UWeaponInventoryItem* WeaponInventoryItem = Cast<UWeaponInventoryItem>(InOperation->Payload);
	if (FWeaponTableRow* WeaponRow = HWDataTableUtils::FindWeaponData(WeaponInventoryItem->GetDataTableID()))
	{
		SpawnItemFromInventory(WeaponRow->PickableActorClass);
		LinkedEquipableItem.Reset();
	}
	else
	{
		LinkedInventoryItem = WeaponInventoryItem;
		if (OnEquipmentDropInSlot.IsBound())
		{
			OnEquipmentDropInSlot.Execute(LinkedInventoryItem->GetEquipWeaponClass(), SlotIndexComponent);
		}
		UE_LOG(LogDataTable, Warning, TEXT("Can't find data about %s"), *LinkedInventoryItem->GetDescription().Name.ToString());
	}
}

void UEquipmentSlotWidget::SpawnItemFromInventory(TSubclassOf<APickableItem> ItemClass)
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!IsValid(OwningPawn))
	{
		return;
	}

	FVector PawnLocation = OwningPawn->GetActorLocation();
	FVector PawnViewDirection = OwningPawn->GetActorRotation().RotateVector(FVector::ForwardVector);
	float OffsetFromPawn = 60.f;
	FVector SpawnLocation = PawnLocation + PawnViewDirection * OffsetFromPawn;
	GetWorld()->SpawnActor(ItemClass, &SpawnLocation, &FRotator::ZeroRotator);
}
