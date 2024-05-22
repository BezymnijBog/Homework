// Fill out your copyright notice in the Description page of Project Settings.


#include "ReticleWidget.h"

#include "Actors/Equipment/EquipableItem.h"
#include "Actors/Equipment/Weapons/Bow.h"

void UReticleWidget::OnAimingStateChanged_Implementation(bool bIsAiming)
{
	SetupCurrentReticle();
}

void UReticleWidget::OnEquippedItemChanged_Implementation(const AEquipableItem* EquippedItem)
{
	CurrentEquippedItem = EquippedItem;
	SetupCurrentReticle();
	if (CurrentEquippedItem.IsValid() && CurrentEquippedItem->IsA<ABow>())
	{
		const ABow* Bow = StaticCast<const ABow*>(CurrentEquippedItem.Get());
		if (!Bow->OnDamageUpdated.IsBoundToObject(this))
		{
			Bow->OnDamageUpdated.BindDynamic(this, &UReticleWidget::OnDamageUpdated);
		}
	}
}

void UReticleWidget::OnDamageUpdated_Implementation(float Percentage)
{
	DamagePercentage = Percentage;
}

void UReticleWidget::SetupCurrentReticle()
{
	CurrentReticle = CurrentEquippedItem.IsValid() ? CurrentEquippedItem->GetReticleType() : EReticleType::None;
}
