// Fill out your copyright notice in the Description page of Project Settings.


#include "PickableAmmo.h"

#include "HomeworkTypes.h"

#include "Characters/HWBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Inventory/Items/Equipables/AmmoInventoryItem.h"
#include "Utils/HWDataTableUtils.h"

APickableAmmo::APickableAmmo()
{
	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>("AmmoMesh");
	SetRootComponent(AmmoMesh);
}

void APickableAmmo::Interact(AHWBaseCharacter* Character)
{
	FAmmoTableRow* AmmoRow = HWDataTableUtils::FindAmmoData(DataTableID);
	if (AmmoRow)
	{
		TWeakObjectPtr<UAmmoInventoryItem> Ammo = NewObject<UAmmoInventoryItem>(Character);
		Ammo->Initialize(DataTableID, AmmoRow->AmmoItemDescription);
		Ammo->SetAmmoType(AmmoRow->AmunitionType);
		Ammo->SetCount(AmmoCount);
		if (Character->PickupItem(Ammo.Get()))
		{
			Character->GetEquipmentComponent_Mutable()->AddAmmo(AmmoRow->AmunitionType, AmmoCount);
			if (!Destroy())
			{
				Character->DestroyActorOnServer(this);
			}
		}
	}
}

FName APickableAmmo::GetActionEventName() const
{
	return ActionInteract;
}
