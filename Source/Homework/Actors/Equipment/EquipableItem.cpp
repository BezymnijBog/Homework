// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipableItem.h"

#include "Characters/HWBaseCharacter.h"

AEquipableItem::AEquipableItem()
{
	SetReplicates(true);
}

void AEquipableItem::Equip()
{
	if (OnEquipmentStateChanged.IsBound())
	{
		OnEquipmentStateChanged.Broadcast(true);
	}
}

void AEquipableItem::UnEquip()
{
	if (OnEquipmentStateChanged.IsBound())
	{
		OnEquipmentStateChanged.Broadcast(false);
	}
}

void AEquipableItem::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	if (IsValid(NewOwner))
	{
		checkf(GetOwner()->IsA<AHWBaseCharacter>(), TEXT("AEquipableItem::SetOwner, Wrong owner class!"));
		CachedBaseCharacterOwner = StaticCast<AHWBaseCharacter*>(NewOwner);
		if (GetLocalRole() == ROLE_Authority)
		{
			SetAutonomousProxy(true);
		}
	}
	else
	{
		CachedBaseCharacterOwner = nullptr;
	}
}

bool AEquipableItem::IsSlotCompatible(EEquipmentSlots Slot) const
{
	return CompatibleSlots.Contains(Slot);
}

AHWBaseCharacter* AEquipableItem::GetBaseCharacterOwner() const
{
	return CachedBaseCharacterOwner.IsValid() ? CachedBaseCharacterOwner.Get() : nullptr;
}

EEquipableItemType AEquipableItem::GetItemType() const
{
	return ItemType;
}

EReticleType AEquipableItem::GetReticleType() const
{
	return ReticleType;
}

FName AEquipableItem::GetDataTableID() const
{
	return DataTableID;
}

FName AEquipableItem::GetEquippedSocketName() const
{
	return EquippedSocketName;
}

FName AEquipableItem::GetUnEquippedSocketName() const
{
	return UnEquippedSocketName;
}

UAnimMontage* AEquipableItem::GetCharacterEquipAnimMontage() const
{
	return CharacterEquipAnimMontage;
}
