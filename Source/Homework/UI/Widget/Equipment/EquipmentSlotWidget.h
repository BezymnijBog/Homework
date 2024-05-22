// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EquipmentSlotWidget.generated.h"

class AEquipableItem;
class APickableItem;
class UImage;
class UInventorySlotWidget;
class UTextBlock;
class UWeaponInventoryItem;

UCLASS()
class HOMEWORK_API UEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DELEGATE_RetVal_TwoParams(bool, FOnEquipmentDropInSlot, const TSubclassOf<AEquipableItem>&, int32);
	DECLARE_DELEGATE_OneParam(FOnEquipmentRemoveFromSlot, int32);

	FOnEquipmentDropInSlot OnEquipmentDropInSlot;
	FOnEquipmentRemoveFromSlot OnEquipmentRemoveFromSlot;

	void InitializeEquipmentSlot(TWeakObjectPtr<AEquipableItem> Equipment, int32 Index);
	void UpdateView();
	
protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY(meta = (BindWidget))
	UImage* ImageWeaponItem;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TBWeaponName;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInventorySlotWidget> DragAndDropWidgetClass;

private:
	void SpawnItemFromInventory(TSubclassOf<APickableItem> ItemClass);
	
	TWeakObjectPtr<AEquipableItem> LinkedEquipableItem;
	TWeakObjectPtr<UWeaponInventoryItem> LinkedInventoryItem;

	int32 SlotIndexComponent = 0;
};
