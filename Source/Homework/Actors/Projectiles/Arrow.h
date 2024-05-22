// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectiles/HWProjectile.h"
#include "Arrow.generated.h"

class APickableItem;
/**
 * 
 */
UCLASS()
class HOMEWORK_API AArrow : public AHWProjectile
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Arrow")
	TSubclassOf<APickableItem> PickableArrowClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Arrow")
	float SpawnLocationCorrection = 50.f;

private:
	UFUNCTION()
	void OnArrowHit(AHWProjectile* Projectile, const FHitResult& HitResult, const FVector& Direction);
};
