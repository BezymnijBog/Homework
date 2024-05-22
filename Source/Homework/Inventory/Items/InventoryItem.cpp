// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItem.h"

void UInventoryItem::Initialize(FName DataTableID_In, const FInventoryItemDescription& ItemDescription)
{
	DataTableID = DataTableID_In;
	Description.Icon = ItemDescription.Icon;
	Description.Name = ItemDescription.Name;
	bIsInitialized = true;
}

int32 UInventoryItem::GetCount() const
{
	return Count;
}

bool UInventoryItem::IsConsumable() const
{
	return bIsConsumable;
}

bool UInventoryItem::IsEquipable() const
{
	return bIsEquipable;
}

bool UInventoryItem::IsStackable() const
{
	return bIsStackable;
}

FName UInventoryItem::GetDataTableID() const
{
	return DataTableID;
}

const FInventoryItemDescription& UInventoryItem::GetDescription() const
{
	return Description;
}
