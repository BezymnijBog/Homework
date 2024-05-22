// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoInventoryItem.h"

UAmmoInventoryItem::UAmmoInventoryItem()
{
	bIsStackable = true;
}

void UAmmoInventoryItem::SetAmmoType(EAmunitionType NewType)
{
	AmmoType = NewType;
}

void UAmmoInventoryItem::SetCount(int32 AmmoCount)
{
	Count = AmmoCount;
}

EAmunitionType UAmmoInventoryItem::GetAmmoType() const
{
	return AmmoType;
}
