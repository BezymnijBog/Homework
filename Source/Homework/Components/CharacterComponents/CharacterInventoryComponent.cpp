// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterInventoryComponent.h"

#include "Inventory/Items/InventoryItem.h"
#include "Inventory/Items/Equipables/AmmoInventoryItem.h"
#include "UI/Widget/Inventory/InventoryViewWidget.h"


bool FInventorySlot::IsEmpty() const
{
	return !Item.IsValid();
}

void FInventorySlot::BindOnInventorySlotUpdate(const FInventorySlotUpdate& Callback) const
{
	OnInventorySlotUpdate = Callback;
}

void FInventorySlot::UnbindOnInventorySlotUpdate()
{
	OnInventorySlotUpdate.Unbind();
}

void FInventorySlot::UpdateSlotState()
{
	bool bExecuted = OnInventorySlotUpdate.ExecuteIfBound();
}

void FInventorySlot::ClearSlot()
{
	Item = nullptr;
	Count = 0;
	UpdateSlotState();
}

void UCharacterInventoryComponent::OpenViewInventory(APlayerController* PlayerController)
{
	if (!IsValid(InventoryViewWidget))
	{
		CreateViewWidget(PlayerController);
	}

	if (!InventoryViewWidget->IsVisible())
	{
		InventoryViewWidget->AddToViewport();
	}
}

void UCharacterInventoryComponent::CloseViewInventory()
{
	if (InventoryViewWidget->IsVisible())
	{
		InventoryViewWidget->RemoveFromParent();
	}
}

void UCharacterInventoryComponent::OnShot(EAmunitionType ConsumedAmmoType)
{
	for (FInventorySlot& Slot : InventorySlots)
	{
		if (!Slot.Item.IsValid())
		{
			continue;
		}
		if (Slot.Item->IsA<UAmmoInventoryItem>() && StaticCast<UAmmoInventoryItem*>(Slot.Item.Get())->GetAmmoType() == ConsumedAmmoType)
		{
			Slot.Count--;
			Slot.UpdateSlotState();
			if (Slot.Count == 0)
			{
				TWeakObjectPtr<UInventoryItem> ItemToRemove = Slot.Item;
				Slot.ClearSlot();
				RemoveItem(ItemToRemove->GetDataTableID());
			}
			break;
		}
	}
}

bool UCharacterInventoryComponent::IsViewVisible() const
{
	return IsValid(InventoryViewWidget) ? InventoryViewWidget->IsVisible() : false;
}

int32 UCharacterInventoryComponent::GetCapacity() const
{
	return Capacity;
}

bool UCharacterInventoryComponent::HasSlotForItem(UInventoryItem* Item) const
{
	bool bHasSlotForStackableItem = false;
	if (Item->IsStackable())
	{
		bHasSlotForStackableItem = (nullptr != FindItemSlot(Item->GetDataTableID()));
	}
	return bHasSlotForStackableItem || ItemsInInventory < Capacity;
}

bool UCharacterInventoryComponent::AddItem(TWeakObjectPtr<UInventoryItem> ItemToAdd, int32 Count)
{
	if (!ItemToAdd.IsValid() || Count < 0)
	{
		return false;
	}

	bool bResult = false;	
	FInventorySlot* FreeSlot = GetSlotForItem(ItemToAdd);

	if (FreeSlot)
	{
		FreeSlot->Count += Count;
		if (FreeSlot->IsEmpty())
		{
			FreeSlot->Item = ItemToAdd;
			ItemsInInventory++;
		}
		bResult = true;
		FreeSlot->UpdateSlotState();
	}

	return bResult;
}

bool UCharacterInventoryComponent::RemoveItem(FName ItemID)
{
	FInventorySlot* ItemSlot = FindItemSlot(ItemID);
	if (ItemSlot)
	{
		auto RemoveCondition = [=](const FInventorySlot& Slot)
		{
			return Slot.Item.IsValid() && Slot.Item->GetDataTableID() == ItemID;
		};
		InventorySlots.RemoveAll(RemoveCondition);
		return true;
	}
	return false;
}

TArray<FInventorySlot> UCharacterInventoryComponent::GetAllItemsCopy() const
{
	return InventorySlots;
}

TArray<FText> UCharacterInventoryComponent::GetAllItemNames() const
{
	TArray<FText> Result;
	for (const FInventorySlot& Slot : InventorySlots)
	{
		if (Slot.Item.IsValid())
		{
			Result.Add(Slot.Item->GetDescription().Name);
		}
	}
	return Result;
}

void UCharacterInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	InventorySlots.AddDefaulted(Capacity);
}

void UCharacterInventoryComponent::CreateViewWidget(APlayerController* PlayerController)
{
	if (!IsValid(PlayerController) || !IsValid(InventoryWidgetClass) || IsValid(InventoryViewWidget))
	{
		return;
	}

	InventoryViewWidget = CreateWidget<UInventoryViewWidget>(PlayerController, InventoryWidgetClass);
	InventoryViewWidget->InitializeViewWidget(InventorySlots);
}

FInventorySlot* UCharacterInventoryComponent::FindItemSlot(FName ItemID)
{
	auto Predicate = [=](const FInventorySlot& Slot)
	{
		return Slot.Item.IsValid() && Slot.Item->GetDataTableID() == ItemID;
	};
	return InventorySlots.FindByPredicate(Predicate);
}

const FInventorySlot* UCharacterInventoryComponent::FindItemSlot(FName ItemID) const
{
	auto Predicate = [=](const FInventorySlot& Slot)
	{
		return Slot.Item.IsValid() && Slot.Item->GetDataTableID() == ItemID;
	};
	return InventorySlots.FindByPredicate(Predicate);
}

FInventorySlot* UCharacterInventoryComponent::FindFreeSlot()
{
	return InventorySlots.FindByPredicate([=](const FInventorySlot& Slot) { return !Slot.Item.IsValid(); });
}

FInventorySlot* UCharacterInventoryComponent::GetSlotForItem(const TWeakObjectPtr<UInventoryItem>& Item)
{
	if (!Item.IsValid())
	{
		return nullptr;
	}
	
	if (Item->IsStackable())
	{
		if (FInventorySlot* Slot = FindItemSlot(Item->GetDataTableID()))
		{
			return Slot;
		}
	}
	return FindFreeSlot();
}
