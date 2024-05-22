// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "HomeworkTypes.h"
#include "Inventory/Items/InventoryItem.h"
#include "AmmoInventoryItem.generated.h"

/**
 * 
 */
UCLASS()
class HOMEWORK_API UAmmoInventoryItem : public UInventoryItem
{
	GENERATED_BODY()

public:
	UAmmoInventoryItem();
	
	void SetAmmoType(EAmunitionType NewType);
	void SetCount(int32 AmmoCount);

	EAmunitionType GetAmmoType() const;

private:
	EAmunitionType AmmoType = EAmunitionType::None;
};
