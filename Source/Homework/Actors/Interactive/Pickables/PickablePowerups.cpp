// Fill out your copyright notice in the Description page of Project Settings.


#include "PickablePowerups.h"

#include "HomeworkTypes.h"
#include "Characters/HWBaseCharacter.h"
#include "Inventory/Items/InventoryItem.h"
#include "Utils/HWDataTableUtils.h"

APickablePowerups::APickablePowerups()
{
	PowerupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PowerupMesh"));
	SetRootComponent(PowerupMesh);
}

void APickablePowerups::Interact(AHWBaseCharacter* Character)
{
	FItemTableRow* ItemData = HWDataTableUtils::FindInventoryItemData(GetDataTableID());

	if (!ItemData)
	{
		return;
	}

	TWeakObjectPtr<UInventoryItem> Item = TWeakObjectPtr<UInventoryItem>(NewObject<UInventoryItem>(Character, ItemData->InventoryItemClass));
	Item->Initialize(DataTableID, ItemData->ItemDescription);

	if (Character->PickupItem(Item.Get()))
	{
		Destroy();
	}
	
}

FName APickablePowerups::GetActionEventName() const
{
	return ActionInteract;
}
