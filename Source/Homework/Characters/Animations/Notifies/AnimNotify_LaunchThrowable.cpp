// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_LaunchThrowable.h"

#include "Characters/HWBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"


void UAnimNotify_LaunchThrowable::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	AHWBaseCharacter* CharacterOwner = Cast<AHWBaseCharacter>(MeshComp->GetOwner());
	if (!IsValid(CharacterOwner))
	{
		return;
	}
	CharacterOwner->GetEquipmentComponent_Mutable()->LaunchCurrentThrowableItem();
}
