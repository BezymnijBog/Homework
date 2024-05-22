// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeaponItem.h"

#include "Characters/HWBaseCharacter.h"
#include "Components/Weapon/MeleeHitRegistrator.h"

AMeleeWeaponItem::AMeleeWeaponItem()
{
	EquippedSocketName = SocketCharacterWeapon;
}

void AMeleeWeaponItem::SetHitRegistrationEnabled(bool bIsEnabled)
{
	HitActors.Empty();
	for (UMeleeHitRegistrator* HitRegistrator : HitRegistrators)
	{
		HitRegistrator->SetHitRegistrationEnabled(bIsEnabled);
	}
}

void AMeleeWeaponItem::StartAttack(EMeleeAttackTypes AttackType)
{
	AHWBaseCharacter* CharacterOwner = GetBaseCharacterOwner();
	if (!IsValid(CharacterOwner))
	{
		return;
	}

	HitActors.Empty();
	FMeleeAttackDescription* Attack = Attacks.Find(AttackType);
	if (Attack && IsValid(Attack->AttackMontage))
	{
		CurrentAttack = Attack;
		UAnimInstance* AnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
		if (IsValid(AnimInstance))
		{
			float Duration = AnimInstance->Montage_Play(Attack->AttackMontage, 1, EMontagePlayReturnType::Duration);
			GetWorld()->GetTimerManager().SetTimer(AttackTimer, this, &AMeleeWeaponItem::OnAttackTimerElapsed, Duration, false);
		}
		else
		{
			OnAttackTimerElapsed();
		}
	}
}

void AMeleeWeaponItem::BeginPlay()
{
	Super::BeginPlay();
	GetComponents<UMeleeHitRegistrator>(HitRegistrators);
	for (UMeleeHitRegistrator* HitRegistrator : HitRegistrators)
	{
		HitRegistrator->OnMeleeHitRegistred.AddUFunction(this, FName("ProcessHit"));
	}
}

void AMeleeWeaponItem::ProcessHit(const FHitResult& HitResult, const FVector& HitDirection)
{
	AActor* HitActor = HitResult.GetActor();
	if (!IsValid(HitActor) || HitActors.Contains(HitActor) || !CurrentAttack)
	{
		return;
	}

	FPointDamageEvent DamageEvent;
	DamageEvent.HitInfo = HitResult;
	DamageEvent.ShotDirection = HitDirection;
	DamageEvent.DamageTypeClass = CurrentAttack->DamageTypeClass;
	AHWBaseCharacter* CharacterOwner = GetBaseCharacterOwner();
	AController* Controller = IsValid(CharacterOwner) ? CharacterOwner->GetController() : nullptr;
	HitActor->TakeDamage(CurrentAttack->DamageAmount, DamageEvent, Controller, GetOwner());

	HitActors.Add(HitActor);
}

void AMeleeWeaponItem::OnAttackTimerElapsed()
{
	CurrentAttack = nullptr;
	SetHitRegistrationEnabled(false);
}
