// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Equipment/EquipableItem.h"
#include "MeleeWeaponItem.generated.h"

class UMeleeHitRegistrator;
USTRUCT(BlueprintType)
struct FMeleeAttackDescription
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee attack")
	TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee attack", meta = (ClampMin = 1.f, UIMin = 1.f))
	float DamageAmount = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee attack")
	UAnimMontage* AttackMontage;
};

UCLASS(Blueprintable)
class HOMEWORK_API AMeleeWeaponItem : public AEquipableItem
{
	GENERATED_BODY()

public:
	AMeleeWeaponItem();

	void SetHitRegistrationEnabled(bool bIsEnabled);
	void StartAttack(EMeleeAttackTypes AttackType);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee attack")
	TMap<EMeleeAttackTypes, FMeleeAttackDescription> Attacks;

private:
	UFUNCTION()
	void ProcessHit(const FHitResult& HitResult, const FVector& HitDirection);

	void OnAttackTimerElapsed();

	FMeleeAttackDescription* CurrentAttack;
	FTimerHandle AttackTimer;
	TArray<UMeleeHitRegistrator*> HitRegistrators;
	TSet<AActor*> HitActors;
};
