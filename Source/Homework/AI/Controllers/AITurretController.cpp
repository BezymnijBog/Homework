// Fill out your copyright notice in the Description page of Project Settings.


#include "AITurretController.h"

#include "AI/Characters/Turret.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Sight.h"

void AAITurretController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	// It's empty, but anyway
	Super::ActorsPerceptionUpdated(UpdatedActors);

	if (!CachedTurret.IsValid())
	{
		return;
	}

	if (!SightUpdate() && !DamageUpdate())
	{
		CachedTurret->CurrentTarget = nullptr;
		CachedTurret->OnCurrentTargetSet();
	}
}

void AAITurretController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	if (IsValid(InPawn))
	{
		checkf(InPawn->IsA<ATurret>(), TEXT("AAITurretController::SetPawn AAITurretController can possess only ATurret"));
		CachedTurret = StaticCast<ATurret*>(InPawn);
	}
	else
	{
		CachedTurret = nullptr;
	}
}

bool AAITurretController::SightUpdate()
{
	AActor* ClosestVisibleActor = GetClosestSensedActor(UAISense_Sight::StaticClass());
	if (IsValid(ClosestVisibleActor))
	{
		CachedTurret->CurrentTarget = ClosestVisibleActor;
		CachedTurret->OnCurrentTargetSet();
		return true;
	}
	return false;
}

bool AAITurretController::DamageUpdate()
{
	AActor* ClosestDamagingActor = GetClosestSensedActor(UAISense_Damage::StaticClass());
	if (IsValid(ClosestDamagingActor))
	{
		CachedTurret->RotateToStimulus(ClosestDamagingActor->GetActorLocation());
		return true;
	}
	return false;
}
