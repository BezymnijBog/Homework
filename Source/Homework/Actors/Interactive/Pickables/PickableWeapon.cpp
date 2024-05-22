// Fill out your copyright notice in the Description page of Project Settings.


#include "PickableWeapon.h"

#include "Characters/HWBaseCharacter.h"
#include "Engine/DataTable.h"
#include "HomeworkTypes.h"
#include "Inventory/Items/InventoryItem.h"
#include "Inventory/Items/Equipables/WeaponInventoryItem.h"
#include "Utils/HWDataTableUtils.h"

APickableWeapon::APickableWeapon()
{
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
}

void APickableWeapon::Interact(AHWBaseCharacter* Character)
{
	FWeaponTableRow* WeaponRow = HWDataTableUtils::FindWeaponData(DataTableID);
	if (WeaponRow)
	{
		TWeakObjectPtr<UWeaponInventoryItem> Weapon = NewObject<UWeaponInventoryItem>(Character);
		Weapon->Initialize(DataTableID, WeaponRow->WeaponItemDescription);
		Weapon->SetEquipWeaponClass(WeaponRow->EquipableActorClass);
		Character->PickupItem(Weapon.Get());
		Destroy();
	}
}

FName APickableWeapon::GetActionEventName() const
{
	return ActionInteract;
}
