// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectiles/HWProjectile.h"
#include "ExplosiveProjectile.generated.h"

class UExplosionComponent;

UENUM(BlueprintType)
enum class EExplosionBehavior : uint8
{
	ByTimer,
	OnHit
};

UCLASS()
class HOMEWORK_API AExplosiveProjectile : public AHWProjectile
{
	GENERATED_BODY()

public:
	AExplosiveProjectile();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void OnProjectileLaunched() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UExplosionComponent* ExplosionComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion")
	EExplosionBehavior ExplosionBehavior = EExplosionBehavior::ByTimer;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Explosion", meta = (EditCondition = "ExplosionBehavior == EExplosionBehavior::ByTimer"))
	float DetonationTime = 2.f;

private:
	void OnDetonationTimerElapsed();
	UFUNCTION()
	void ExplodeOnHit(AHWProjectile* Projectile, const FHitResult& HitResult, const FVector& Direction);

	UFUNCTION(Server, Reliable)
	void Server_Explode();

	void ExplodeInternal();
	
	AController* GetController();

	UPROPERTY(Replicated)
	FVector_NetQuantize100 ExplodeLocation_Mul_10;
	
	UPROPERTY(ReplicatedUsing=OnRep_HaveToExplode)
	bool bHaveToExplode = false;

	UFUNCTION()
	void OnRep_HaveToExplode(bool bHaveToExplode_Old);
	
	FTimerHandle DetonationTimer;
};
