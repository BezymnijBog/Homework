// Fill out your copyright notice in the Description page of Project Settings.


#include "Adrenaline.h"

#include "Characters/HWBaseCharacter.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"

bool UAdrenaline::Consume(AHWBaseCharacter* ConsumeTarget)
{
	UCharacterAttributesComponent* AttributesComponent = ConsumeTarget->GetCharacterAttributes_Mutable();
	AttributesComponent->AddStamina(Stamina);
	ConditionalBeginDestroy();
	return true;
}
