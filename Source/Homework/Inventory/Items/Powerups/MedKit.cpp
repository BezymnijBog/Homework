// Fill out your copyright notice in the Description page of Project Settings.


#include "MedKit.h"

#include "Characters/HWBaseCharacter.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"

bool UMedKit::Consume(AHWBaseCharacter* ConsumeTarget)
{
	UCharacterAttributesComponent* AttributesComponent = ConsumeTarget->GetCharacterAttributes_Mutable();
	AttributesComponent->AddHealth(Health);
	ConditionalBeginDestroy();
	return true;
}
