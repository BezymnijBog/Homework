// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponWheelWidget.h"


#include "Actors/Equipment/EquipableItem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Inventory/Items/InventoryItem.h"
#include "Utils/HWDataTableUtils.h"

void UWeaponWheelWidget::InitializeWeaponWheelWidget(UCharacterEquipmentComponent* EquipmentComponent)
{
	LinkedEquipmentComponent = EquipmentComponent;
}

void UWeaponWheelWidget::ConfirmSelection()
{
	EEquipmentSlots SelectedSlot = EquipmentSlotSegments[CurrentSegmentIndex];
	LinkedEquipmentComponent->EquipItemInSlot(SelectedSlot);
	RemoveFromParent();
}

void UWeaponWheelWidget::NextSegment()
{
	CurrentSegmentIndex++;
	if (CurrentSegmentIndex >= EquipmentSlotSegments.Num())
	{
		CurrentSegmentIndex = 0;
	}
	SelectSegment();
}

void UWeaponWheelWidget::PreviousSegment()
{
	CurrentSegmentIndex--;
	if (CurrentSegmentIndex < 0)
	{
		CurrentSegmentIndex = EquipmentSlotSegments.Num() - 1;
	}
	SelectSegment();
}

void UWeaponWheelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (IsValid(RadialBackground) && !BackgroundMaterial.IsValid())
	{
		BackgroundMaterial = RadialBackground->GetDynamicMaterial();
		BackgroundMaterial->SetScalarParameterValue(FName("Segments"), EquipmentSlotSegments.Num());	
	}

	for (int i = 0; i < EquipmentSlotSegments.Num(); ++i)
	{
		FName WidgetName = FName(FString::Printf(TEXT("Image_Segment%i"), i));
		UImage* WeaponImage = WidgetTree->FindWidget<UImage>(WidgetName);
		if (IsValid(WeaponImage))
		{
			FWeaponTableRow* WeaponData = GetTableRowForSegment(i);
			if (WeaponData)
			{
				WeaponImage->SetOpacity(1.f);
				WeaponImage->SetBrushFromTexture(WeaponData->WeaponItemDescription.Icon);
			}
			else
			{
				WeaponImage->SetOpacity(0.f);
			}
		}
	}
}

void UWeaponWheelWidget::SelectSegment()
{
	BackgroundMaterial->SetScalarParameterValue(FName("Index"), CurrentSegmentIndex);
	FWeaponTableRow* WeaponData = GetTableRowForSegment(CurrentSegmentIndex);
	if (!WeaponData)
	{
		WeaponNameText->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		WeaponNameText->SetVisibility(ESlateVisibility::Visible);
		WeaponNameText->SetText(WeaponData->WeaponItemDescription.Name);
	}
}

FWeaponTableRow* UWeaponWheelWidget::GetTableRowForSegment(int32 SegmentIndex) const
{
	const EEquipmentSlots& SlotSegment = EquipmentSlotSegments[SegmentIndex];
	int32 SlotIndex = StaticCast<int32>(SlotSegment);
	AEquipableItem* EquipableItem = LinkedEquipmentComponent->GetItems()[SlotIndex];
	if (!IsValid(EquipableItem))
	{
		return nullptr;
	}
	return HWDataTableUtils::FindWeaponData(EquipableItem->GetDataTableID());
}
