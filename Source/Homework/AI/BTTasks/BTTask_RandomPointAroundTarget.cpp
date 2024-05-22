// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_RandomPointAroundTarget.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

#define CheckTaskFail(expr) {if (expr) return EBTNodeResult::Failed; }

UBTTask_RandomPointAroundTarget::UBTTask_RandomPointAroundTarget()
{
    NodeName = "Random point around target";
}

EBTNodeResult::Type UBTTask_RandomPointAroundTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    CheckTaskFail(IsValid(AIController) && IsValid(Blackboard))

    APawn* Pawn = AIController->GetPawn();
    CheckTaskFail(IsValid(Pawn))

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Pawn);
    CheckTaskFail(IsValid(NavSys))

    AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetKey.SelectedKeyName));
    CheckTaskFail(IsValid(TargetActor))

    FNavLocation NavLocation;
    bool bIsFound = NavSys->GetRandomReachablePointInRadius(TargetActor->GetActorLocation(), Radius, NavLocation);
    CheckTaskFail(bIsFound)

    Blackboard->SetValueAsVector(LocationKey.SelectedKeyName, NavLocation.Location);
    return EBTNodeResult::Succeeded;
}
