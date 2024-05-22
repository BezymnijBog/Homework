// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_SetMeleHitRegEnabled.h"


#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "Characters/HWBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void UAnimNotify_SetMeleHitRegEnabled::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    Super::Notify(MeshComp, Animation);
	AHWBaseCharacter* CharacterOwner = Cast<AHWBaseCharacter>(MeshComp->GetOwner());
	if (!IsValid(CharacterOwner))
	{
		return;
	}
	AMeleeWeaponItem* MeleeWeapon = CharacterOwner->GetEquipmentComponent()->GetCurrentMeleeWeapon();
	if (IsValid(MeleeWeapon))
	{
		MeleeWeapon->SetHitRegistrationEnabled(bIsHitRegistrationEnabled);
	}
}
