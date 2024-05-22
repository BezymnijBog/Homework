// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractiveActor.h"

#include "Homework/Characters/HWBaseCharacter.h"
#include "Components/CapsuleComponent.h"


void AInteractiveActor::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(InteractionVolume))
	{
		InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &AInteractiveActor::OnInteractionVolumeBeginOverlap);
		InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &AInteractiveActor::OnInteractionVolumeEndOverlap);
	}
}

bool AInteractiveActor::IsOverlappingCharacterCapsule(AActor* OtherActor, UPrimitiveComponent* OtherComp) const
{
	AHWBaseCharacter* BaseCharacter = Cast<AHWBaseCharacter>(OtherActor);
	if (!IsValid(BaseCharacter))
	{
		return false;
	}
	if (BaseCharacter->GetCapsuleComponent() != Cast<UCapsuleComponent>(OtherComp))
	{
		return false;
	}
	return true;
}

void AInteractiveActor::OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsOverlappingCharacterCapsule(OtherActor, OtherComp))
	{
		return;
	}
	AHWBaseCharacter* BaseCharacter = StaticCast<AHWBaseCharacter*>(OtherActor);
	BaseCharacter->RegisterInteractiveActor(this);
}

void AInteractiveActor::OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsOverlappingCharacterCapsule(OtherActor, OtherComp))
	{
		return;
	}
	AHWBaseCharacter* BaseCharacter = StaticCast<AHWBaseCharacter*>(OtherActor);
	BaseCharacter->UnRegisterInteractiveActor(this);
}

