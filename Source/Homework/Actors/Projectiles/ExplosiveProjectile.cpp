// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplosiveProjectile.h"

#include "Components/ExplosionComponent.h"
#include "Net/UnrealNetwork.h"

AExplosiveProjectile::AExplosiveProjectile()
{
	ExplosionComponent = CreateDefaultSubobject<UExplosionComponent>(TEXT("Explosion Component"));
	ExplosionComponent->SetupAttachment(GetRootComponent());
}

void AExplosiveProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AExplosiveProjectile, bHaveToExplode);
	DOREPLIFETIME(AExplosiveProjectile, ExplodeLocation_Mul_10);
}

void AExplosiveProjectile::OnProjectileLaunched()
{
	Super::OnProjectileLaunched();
	switch (ExplosionBehavior)
	{
	case EExplosionBehavior::ByTimer:
		GetWorld()->GetTimerManager().SetTimer(DetonationTimer, this, &AExplosiveProjectile::OnDetonationTimerElapsed, DetonationTime);
		break;
	case EExplosionBehavior::OnHit:
		OnProjectileHit.AddDynamic(this, &AExplosiveProjectile::ExplodeOnHit);
		break;
	}
	bHaveToExplode = false;
}

void AExplosiveProjectile::OnDetonationTimerElapsed()
{
	Server_Explode();
}

void AExplosiveProjectile::ExplodeOnHit(AHWProjectile* Projectile, const FHitResult& HitResult, const FVector& Direction)
{
	Server_Explode();
	OnProjectileHit.Remove(this, FName("ExplodeOnHit"));
}

void AExplosiveProjectile::Server_Explode_Implementation()
{
	bHaveToExplode = true;
	ExplodeLocation_Mul_10 = GetActorLocation() * 10.f;
	ExplodeInternal();
}

void AExplosiveProjectile::ExplodeInternal()
{
	ExplosionComponent->Explode(GetController(), ExplodeLocation_Mul_10 * 0.1f);
}

AController* AExplosiveProjectile::GetController()
{
	if (GetOwner()->IsA<APawn>())
	{
		return StaticCast<APawn*>(GetOwner())->GetController();
	}
	return nullptr;
}

void AExplosiveProjectile::OnRep_HaveToExplode(bool bHaveToExplode_Old)
{
	if (bHaveToExplode && !bHaveToExplode_Old)
	{
		ExplodeInternal();
	}
}
