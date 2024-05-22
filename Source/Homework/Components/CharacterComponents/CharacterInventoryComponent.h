// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


#include "HomeworkTypes.h"
#include "Components/ActorComponent.h"
#include "CharacterInventoryComponent.generated.h"

class UInventoryViewWidget;
class UInventoryItem;

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

	DECLARE_DELEGATE(FInventorySlotUpdate)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UInventoryItem> Item;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 0;

	bool IsEmpty() const;
	void BindOnInventorySlotUpdate(const FInventorySlotUpdate& Callback) const;
	void UnbindOnInventorySlotUpdate();
	void UpdateSlotState();
	void ClearSlot();

private:
	mutable FInventorySlotUpdate OnInventorySlotUpdate;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOMEWORK_API UCharacterInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	void OpenViewInventory(APlayerController* PlayerController);
	void CloseViewInventory();
	
	void OnShot(EAmunitionType ConsumedAmmoType);
	bool IsViewVisible() const;

	int32 GetCapacity() const;
	bool HasSlotForItem(UInventoryItem* Item) const;

	bool AddItem(TWeakObjectPtr<UInventoryItem> ItemToAdd, int32 Count);
	bool RemoveItem(FName ItemID);

	TArray<FInventorySlot> GetAllItemsCopy() const;
	TArray<FText> GetAllItemNames() const;

protected:
	virtual void BeginPlay() override;

	void CreateViewWidget(APlayerController* PlayerController);

	FInventorySlot* FindItemSlot(FName ItemID);
	// overload for const methods
	const FInventorySlot* FindItemSlot(FName ItemID) const; 

	FInventorySlot* FindFreeSlot();

	FInventorySlot* GetSlotForItem(const TWeakObjectPtr<UInventoryItem>& Item);

	UPROPERTY(EditAnywhere, Category = "Items")
	TArray<FInventorySlot> InventorySlots;

	UPROPERTY(EditAnywhere, Category = "View settings")
	TSubclassOf<UInventoryViewWidget> InventoryWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory settings", meta = (ClampMin = 1, UIMin = 1))
	int32 Capacity = 16;

private:
	UPROPERTY()
	UInventoryViewWidget* InventoryViewWidget;

	int32 ItemsInInventory;
};
