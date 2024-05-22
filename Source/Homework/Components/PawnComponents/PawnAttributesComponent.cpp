// Fill out your copyright notice in the Description page of Project Settings.


#include "PawnAttributesComponent.h"

#include "Net/UnrealNetwork.h"

UPawnAttributesComponent::UPawnAttributesComponent()
{
	SetIsReplicatedByDefault(true);
}

void UPawnAttributesComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPawnAttributesComponent, Health);
}

bool UPawnAttributesComponent::IsAlive() const
{
	return Health > 0.f;
}

void UPawnAttributesComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	if (GetOwner()->HasAuthority())
	{
		GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UPawnAttributesComponent::OnTakeAnyDamage);
	}
}

void UPawnAttributesComponent::OnRep_Health()
{
	OnHealthChanged();
}

void UPawnAttributesComponent::OnHealthChanged()
{
	if (Health <= 0.f && OnDeathDelegate.IsBound())
	{
		OnDeathDelegate.Broadcast();
	}
}

void UPawnAttributesComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (!IsAlive())
	{
		return;
	}
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	OnHealthChanged();
}

