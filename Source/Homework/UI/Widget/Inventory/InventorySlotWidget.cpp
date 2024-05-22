// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlotWidget.h"


#include "Actors/Interactive/Pickables/PickableItem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Characters/HWBaseCharacter.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"
#include "Inventory/Items/InventoryItem.h"
#include "Utils/HWDataTableUtils.h"

void UInventorySlotWidget::InitializeItemSlot(FInventorySlot& InventorySlot)
{
	LinkedSlot = &InventorySlot;

	FInventorySlot::FInventorySlotUpdate OnInventorySlotUpdate;
	OnInventorySlotUpdate.BindUObject(this, &UInventorySlotWidget::UpdateView);
	LinkedSlot->BindOnInventorySlotUpdate(OnInventorySlotUpdate);
}

void UInventorySlotWidget::UpdateView()
{
	if (LinkedSlot && LinkedSlot->Item.IsValid())
	{
		const FInventoryItemDescription& Description = LinkedSlot->Item->GetDescription();
		ImageItemIcon->SetBrushFromTexture(Description.Icon);
		TextItemCount->SetText(FText::AsNumber(LinkedSlot->Count));
	}
	else
	{
		TextItemCount->SetText(FText());
		ImageItemIcon->SetBrushFromTexture(nullptr);
	}
}

void UInventorySlotWidget::SetItemIcon(UTexture2D* Icon)
{
	ImageItemIcon->SetBrushFromTexture(Icon);
}

void UInventorySlotWidget::SetItemCount(int32 Count)
{
	TextItemCount->SetText(FText::AsNumber(Count));
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!LinkedSlot || !LinkedSlot->Item.IsValid())
	{
		return FReply::Handled();
	}
	
	FKey MouseBtn = InMouseEvent.GetEffectingButton();
	if (MouseBtn == EKeys::RightMouseButton)
	{
		TWeakObjectPtr<UInventoryItem> LinkedSlotItem = LinkedSlot->Item;
		AHWBaseCharacter* ItemOwner = Cast<AHWBaseCharacter>(LinkedSlotItem->GetOuter());

		if (LinkedSlotItem->Consume(ItemOwner))
		{
			LinkedSlot->ClearSlot();
		}
		return FReply::Handled();
	}

	FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
	return Reply.NativeReply;
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	UDragDropOperation* DragOperation = Cast<UDragDropOperation>(UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass()));

	UInventorySlotWidget* DragWidget = CreateWidget<UInventorySlotWidget>(GetOwningPlayer(), GetClass());
	DragWidget->ImageItemIcon->SetBrushFromTexture(LinkedSlot->Item->GetDescription().Icon);
	DragWidget->TextItemCount->SetText(FText::AsNumber(LinkedSlot->Count));

	DragOperation->DefaultDragVisual = DragWidget;
	DragOperation->Pivot = EDragPivot::MouseDown;
	DragOperation->Payload = LinkedSlot->Item.Get();
	OutOperation = DragOperation;

	LinkedSlot->ClearSlot();	
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (LinkedSlot && !LinkedSlot->Item.IsValid())
	{
		LinkedSlot->Item = TWeakObjectPtr<UInventoryItem>(Cast<UInventoryItem>(InOperation->Payload));
		LinkedSlot->Count = FCString::Atoi(*Cast<UInventorySlotWidget>(InOperation->DefaultDragVisual)->TextItemCount->Text.ToString());
		LinkedSlot->UpdateSlotState();
		return true;
	}
	
	return false;
}

void UInventorySlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// Spawn object in world
	TWeakObjectPtr<UInventoryItem> Item = MakeWeakObjectPtr(Cast<UInventoryItem>(InOperation->Payload));
	if (FWeaponTableRow* WeaponRow = HWDataTableUtils::FindWeaponData(Item->GetDataTableID()))
	{
		SpawnItemFromInventory(WeaponRow->PickableActorClass);
		LinkedSlot->ClearSlot();
	}
	else if (FItemTableRow* ItemRow = HWDataTableUtils::FindInventoryItemData(Item->GetDataTableID()))
	{
		SpawnItemFromInventory(ItemRow->PickableActorClass);
		LinkedSlot->ClearSlot();
	}
	else if (FAmmoTableRow* AmmoRow = HWDataTableUtils::FindAmmoData(Item->GetDataTableID()))
	{
		SpawnItemFromInventory(AmmoRow->PickableActorClass);
		AHWBaseCharacter* Character = Cast<AHWBaseCharacter>(GetOwningPlayerPawn());
		Character->GetEquipmentComponent_Mutable()->RemoveAmmo(AmmoRow->AmunitionType);
		LinkedSlot->ClearSlot();
	}
	else
	{
		LinkedSlot->Item = TWeakObjectPtr<UInventoryItem>(Cast<UInventoryItem>(InOperation->Payload));
		LinkedSlot->UpdateSlotState();
		UE_LOG(LogDataTable, Warning, TEXT("Can't find data about %s"), *LinkedSlot->Item->GetDescription().Name.ToString());
	}
}

void UInventorySlotWidget::SpawnItemFromInventory(TSubclassOf<APickableItem> ItemClass)
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
