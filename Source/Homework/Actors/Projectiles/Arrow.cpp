// Fill out your copyright notice in the Description page of Project Settings.


#include "Arrow.h"

#include "Actors/Interactive/Pickables/PickableItem.h"

void AArrow::BeginPlay()
{
	Super::BeginPlay();
	OnProjectileHit.AddDynamic(this, &AArrow::OnArrowHit);
}

void AArrow::OnArrowHit(AHWProjectile* Projectile, const FHitResult& HitResult, const FVector& Direction)
{
	if (!HasAuthority())
	{
		return;
	}
	FVector SpawnLocation = HitResult.ImpactPoint;
	SpawnLocation -= Direction * SpawnLocationCorrection;
	FRotator SpawnRotation = Direction.ToOrientationRotator();
	SpawnRotation.Pitch -= 90.f;
	AActor* PickableArrow = GetWorld()->SpawnActor(PickableArrowClass, &SpawnLocation, &SpawnRotation);
	PickableArrow->SetAutonomousProxy(true);
	if (AActor* HitActor = HitResult.GetActor())
	{
		TArray<UActorComponent*> MeshComponents;
		HitActor->GetComponents(UMeshComponent::StaticClass(), MeshComponents);
		if (MeshComponents.Num() > 0)
		{
			PickableArrow->AttachToComponent(StaticCast<USceneComponent*>(MeshComponents[0]), FAttachmentTransformRules::KeepWorldTransform);
		}
	}
}
