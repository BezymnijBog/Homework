// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryViewWidget.generated.h"

struct FInventorySlot;

class UGridPanel;
class UImage;
class UInventorySlotWidget;

UCLASS()
class HOMEWORK_API UInventoryViewWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeViewWidget(TArray<FInventorySlot>& InventorySlots);

protected:
	UPROPERTY(meta = (BindWidget))
	UGridPanel* GridPanelItemSlots;

	UPROPERTY(EditDefaultsOnly, Category = "Item container | View settings")
	TSubclassOf<UInventorySlotWidget> InventorySlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Item container | View settings")
	int32 ColumnCount = 4;

	void AddItemSlotView(FInventorySlot& SlotToAdd);
	
};
