// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_ChangeMovementMode.h"

#include "Characters/HWBaseCharacter.h"

void UAnimNotify_ChangeMovementMode::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	AHWBaseCharacter* CharacterOwner = Cast<AHWBaseCharacter>(MeshComp->GetOwner());
	if (!IsValid(CharacterOwner))
	{
		return;
	}
	CharacterOwner->GetCharacterMovement()->SetMovementMode(NewMovementMode, CustomMovementMode);
	CharacterOwner->bUseControllerRotationYaw = bUseControllerRotationYaw;
}
