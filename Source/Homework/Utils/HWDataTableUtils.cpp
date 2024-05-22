// Fill out your copyright notice in the Description page of Project Settings.


#include "HWDataTableUtils.h"

#include "Inventory/Items/InventoryItem.h"


namespace
{
	template <typename T>
	T* FindTableRowData(const FName& RowID, const FString& ContextConfig, const TCHAR* Name)
	{
		UDataTable* DataTable = LoadObject<UDataTable>(nullptr, Name);

		if (!DataTable)
		{
			return nullptr;
		}

		return DataTable->FindRow<T>(RowID, ContextConfig);
	}
}

FWeaponTableRow* HWDataTableUtils::FindWeaponData(FName WeaponID)
{
	static const FString ContextConfig(TEXT("Find Weapon Data"));
	return FindTableRowData<FWeaponTableRow>(WeaponID, ContextConfig, TEXT("/Game/Homework/Core/Data/DataTables/DT_WeaponList.DT_WeaponList"));
}

FItemTableRow* HWDataTableUtils::FindInventoryItemData(FName ItemID)
{
	static const FString ContextConfig(TEXT("Find Item Data"));
	return FindTableRowData<FItemTableRow>(ItemID, ContextConfig, TEXT("/Game/Homework/Core/Data/DataTables/DT_InventoryItemList.DT_InventoryItemList"));
}

FAmmoTableRow* HWDataTableUtils::FindAmmoData(FName AmmoID)
{
	static const FString ContextConfig(TEXT("Find Ammo Data"));
	return FindTableRowData<FAmmoTableRow>(AmmoID, ContextConfig, TEXT("/Game/Homework/Core/Data/DataTables/DT_AmmoList.DT_AmmoList"));
}

FAmmoTableRow* HWDataTableUtils::FindAmmoData(EAmunitionType AmmoType, FName& AmmoID)
{
	static const FString ContextConfig(TEXT("Find Ammo Data"));
	UDataTable* DataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Homework/Core/Data/DataTables/DT_AmmoList.DT_AmmoList"));
	for (FName RowID : DataTable->GetRowNames())
	{
		if (FAmmoTableRow* AmmoRow = DataTable->FindRow<FAmmoTableRow>(RowID, ContextConfig))
		{
			if (AmmoRow->AmunitionType == AmmoType)
			{
				AmmoID = RowID;
				return AmmoRow;
			}
		}
	}
	return nullptr;
}
