// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/Items/InventoryItem.h"

#include "InventorySlotWidget.generated.h"

class UTextBlock;
struct FInventorySlot;
class UImage;

UCLASS()
class HOMEWORK_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeItemSlot(FInventorySlot& InventorySlot);
	void UpdateView();
	void SetItemIcon(UTexture2D* Icon);
	void SetItemCount(int32 Count);

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ImageItemIcon;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextItemCount;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	void SpawnItemFromInventory(TSubclassOf<APickableItem> ItemClass);
	
	FInventorySlot* LinkedSlot;
};
