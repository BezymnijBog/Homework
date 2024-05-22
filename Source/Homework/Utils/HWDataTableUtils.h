// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "HomeworkTypes.h"

struct FAmmoTableRow;
struct FItemTableRow;
struct FWeaponTableRow;

namespace HWDataTableUtils
{
	FWeaponTableRow* FindWeaponData(FName WeaponID);
	FItemTableRow* FindInventoryItemData(FName ItemID);
	FAmmoTableRow* FindAmmoData(FName AmmoID);
	FAmmoTableRow* FindAmmoData(EAmunitionType AmmoType, FName& AmmoID);
}
