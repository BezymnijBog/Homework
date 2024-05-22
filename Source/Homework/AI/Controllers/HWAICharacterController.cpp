// Fill out your copyright notice in the Description page of Project Settings.


#include "HWAICharacterController.h"

#include "AI/Characters/HWAICharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CharacterComponents/AIPatrollingComponent.h"
#include "Perception/AISense_Sight.h"

void AHWAICharacterController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	Super::ActorsPerceptionUpdated(UpdatedActors);
	if (!CachedCharacter.IsValid())
	{
		return;
	}

	TryMoveToNextTarget();
}

void AHWAICharacterController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	if (!Result.IsSuccess())
	{
		return;
	}

	TryMoveToNextTarget();
}

void AHWAICharacterController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	if (IsValid(InPawn))
	{
		checkf(InPawn->IsA<AHWAICharacter>(), TEXT("AHWAICharacterController::SetPawn, AICharacterController can possess only HWAICharacter"));
		CachedCharacter = StaticCast<AHWAICharacter*>(InPawn);
		RunBehaviorTree(CachedCharacter->GetBehaviorTree());
		SetupPatrolling();
	}
	else
	{
		CachedCharacter = nullptr;
	}
}

void AHWAICharacterController::SetupPatrolling()
{
	UAIPatrollingComponent* PatrollingComponent = CachedCharacter->GetAIPatrolling();
	if (IsValid(PatrollingComponent) && PatrollingComponent->CanPatrol())
	{
		FVector ClosestWayPoint = PatrollingComponent->SelectClosestWayPoint();
		if (IsValid(Blackboard))
		{
			Blackboard->SetValueAsVector(BB_NextLocation, ClosestWayPoint);
			Blackboard->SetValueAsObject(BB_CurrentTraget, nullptr);
		}
		bIsPatrolling = true;
	}
}

void AHWAICharacterController::TryMoveToNextTarget()
{
	UAIPatrollingComponent* PatrollingComponent = CachedCharacter->GetAIPatrolling();
	AActor* ClosestActor = GetClosestSensedActor(UAISense_Sight::StaticClass());
	if (IsValid(ClosestActor))
	{
		if (IsValid(Blackboard))
		{
			Blackboard->SetValueAsObject(BB_CurrentTraget, ClosestActor);
			SetFocus(ClosestActor, EAIFocusPriority::Gameplay);
		}
		bIsPatrolling = false;
	}
	else if (PatrollingComponent->CanPatrol())
	{
		FVector WayPoint = bIsPatrolling ? PatrollingComponent->SelectNextWayPoint() : PatrollingComponent->SelectClosestWayPoint();
		if (!IsTargetReached(WayPoint))
		{
			if (IsValid(Blackboard))
			{
				ClearFocus(EAIFocusPriority::Gameplay);
				Blackboard->SetValueAsVector(BB_NextLocation, WayPoint);
				Blackboard->SetValueAsObject(BB_CurrentTraget, nullptr);
			}
		}
		bIsPatrolling = true;
	}
}

bool AHWAICharacterController::IsTargetReached(const FVector& TargetLocation) const
{
	return (TargetLocation - CachedCharacter->GetActorLocation()).SizeSquared() <= FMath::Square(TargetReachRadius);
}
