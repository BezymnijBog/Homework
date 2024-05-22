// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Items/InventoryItem.h"
#include "MedKit.generated.h"

/**
 * 
 */
UCLASS()
class HOMEWORK_API UMedKit : public UInventoryItem
{
	GENERATED_BODY()

public:
	virtual bool Consume(AHWBaseCharacter* ConsumeTarget) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Medkit")
	float Health = 30.f;
	
};