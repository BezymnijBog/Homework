// Fill out your copyright notice in the Description page of Project Settings.


#include "HWAIController.h"

#include "Perception/AIPerceptionComponent.h"


AHWAIController::AHWAIController()
{
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    
}

AActor* AHWAIController::GetClosestSensedActor(TSubclassOf<UAISense> SenseClass)
{
	if (!IsValid(GetPawn()))
	{
		return nullptr;
	}

	TArray<AActor*> SencedActors;
	PerceptionComponent->GetCurrentlyPerceivedActors(SenseClass, SencedActors);

	AActor* ClosestActor = nullptr;
	float MinSquaredDistance = FLT_MAX;
	FVector PawnLocation = GetPawn()->GetActorLocation();
	for (AActor* SencedActor : SencedActors)
	{
		float CurrentSquaredDistance = FVector::DistSquared(PawnLocation, SencedActor->GetActorLocation());
		if (CurrentSquaredDistance < MinSquaredDistance)
		{
			MinSquaredDistance = CurrentSquaredDistance;
			ClosestActor = SencedActor;
		}
	}
	return ClosestActor;
}
