// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "HomeworkTypes.h"
#include "Blueprint/UserWidget.h"
#include "WeaponWheelWidget.generated.h"

class UCharacterEquipmentComponent;
class UImage;
class UTextBlock;

struct FWeaponTableRow;

UCLASS()
class HOMEWORK_API UWeaponWheelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeWeaponWheelWidget(UCharacterEquipmentComponent* EquipmentComponent);

	void ConfirmSelection();
	void NextSegment();
	void PreviousSegment();
	
protected:
	virtual void NativeConstruct() override;
	void SelectSegment();
	
	UPROPERTY(meta = (BindWidget))
	UImage* RadialBackground;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponNameText;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon wheel settings")
	TArray<EEquipmentSlots> EquipmentSlotSegments;

private:
	FWeaponTableRow* GetTableRowForSegment(int32 SegmentIndex) const;
	
	int32 CurrentSegmentIndex;
	TWeakObjectPtr<UMaterialInstanceDynamic> BackgroundMaterial;
	TWeakObjectPtr<UCharacterEquipmentComponent> LinkedEquipmentComponent;
};
