// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "HomeworkTypes.h"

#include "Engine/DataTable.h"
#include "UObject/NoExportTypes.h"
#include "InventoryItem.generated.h"

class AEquipableItem;
class AHWBaseCharacter;
class APickableItem;
class UInventoryItem;

USTRUCT(BlueprintType)
struct FInventoryItemDescription : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item view")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item view")
	UTexture2D* Icon;
};

USTRUCT(BlueprintType)
struct FWeaponTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon view")
	TSubclassOf<APickableItem> PickableActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon view")
	TSubclassOf<AEquipableItem> EquipableActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon view")
	FInventoryItemDescription WeaponItemDescription;	
};

USTRUCT(BlueprintType)
struct FItemTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item view")
	TSubclassOf<APickableItem> PickableActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item view")
	TSubclassOf<UInventoryItem> InventoryItemClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item view")
	FInventoryItemDescription ItemDescription;
};

USTRUCT(BlueprintType)
struct FAmmoTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item view")
	TSubclassOf<APickableItem> PickableActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item view")
	EAmunitionType AmunitionType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item view")
	FInventoryItemDescription AmmoItemDescription;
};

UCLASS(Blueprintable)
class HOMEWORK_API UInventoryItem : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(FName DataTableID_In, const FInventoryItemDescription& ItemDescription);

	int32 GetCount() const;
	virtual bool IsConsumable() const;
	virtual bool IsEquipable() const;
	virtual bool IsStackable() const;
	virtual bool Consume(AHWBaseCharacter* ConsumeTarget) PURE_VIRTUAL(UInventoryItem::Consume, return false;);
	
	FName GetDataTableID() const;

	const FInventoryItemDescription& GetDescription() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory item")
	FName DataTableID = NAME_None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory item")
	FInventoryItemDescription Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory item")
	bool bIsEquipable = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory item")
	bool bIsConsumable = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory item")
	bool bIsStackable = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory item")
	int32 Count = 1;

private:
	bool bIsInitialized = false;
};
