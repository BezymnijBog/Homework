// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EquipmentViewWidget.generated.h"

class AEquipableItem;
class UCharacterEquipmentComponent;
class UEquipmentSlotWidget;
class UVerticalBox;

UCLASS()
class HOMEWORK_API UEquipmentViewWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeEquipmentWidget(UCharacterEquipmentComponent* EquipmentComponent);

protected:
	void AddEquipmentSlotView(AEquipableItem* LinkToEquipment, int32 SlotIndex);
	void UpdateSlots(int32 SlotIndex);

	bool EquipEquipmentToSlot(const TSubclassOf<AEquipableItem>& EquipmentClass, int32 SenderIndex);
	void RemoveEquipmentFromSlot(int32 SlotIndex);

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* VBEquipmentSlots;

	UPROPERTY(EditDefaultsOnly, Category= "Items container | View settings")
	TSubclassOf<UEquipmentSlotWidget> DefaultSlotViewClass;

	TWeakObjectPtr<UCharacterEquipmentComponent> LinkedEquipmentComponent;
};
