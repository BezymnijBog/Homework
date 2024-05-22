// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_StopSlide.h"

#include "Characters/HWBaseCharacter.h"

void UAnimNotify_StopSlide::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	check(MeshComp->GetOwner()->IsA<AHWBaseCharacter>());
	AHWBaseCharacter* BaseCharacter = StaticCast<AHWBaseCharacter*>(MeshComp->GetOwner());
	BaseCharacter->GetBaseCharacterMovement()->StopSliding();
}
