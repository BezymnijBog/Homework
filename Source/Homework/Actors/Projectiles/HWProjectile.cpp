// Fill out your copyright notice in the Description page of Project Settings.


#include "HWProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AHWProjectile::AHWProjectile()
{
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Component"));
	CollisionComponent->InitSphereRadius(5.f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// TODO make own collision preset
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	SetRootComponent(CollisionComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->bAutoActivate = false;

	SetReplicates(true);
	AActor::SetReplicateMovement(true);
}

void AHWProjectile::SetInitialSpped(float Speed)
{
	if (IsValid(ProjectileMovement))
	{
		ProjectileMovement->InitialSpeed = Speed;
	}
}

void AHWProjectile::SetReturnLocation(const FVector& Location)
{
	ProjectileReturnLocation = Location;
}

void AHWProjectile::LaunchProjectile(const FVector& Direction)
{
	ProjectileMovement->Velocity = Direction * ProjectileMovement->InitialSpeed;
	CollisionComponent->IgnoreActorWhenMoving(GetOwner(), true);
	OnProjectileLaunched();
}

void AHWProjectile::SetProjectileActive_Implementation(bool bIsActive)
{
	ProjectileMovement->SetActive(bIsActive, true);
}

void AHWProjectile::BeginPlay()
{
	Super::BeginPlay();
	CollisionComponent->OnComponentHit.AddDynamic(this, &AHWProjectile::OnCollisionHit);
}

void AHWProjectile::OnProjectileLaunched()
{
}

void AHWProjectile::OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OnProjectileHit.IsBound())
	{
		OnProjectileHit.Broadcast(this, Hit, ProjectileMovement->Velocity.GetSafeNormal());
	}
}

