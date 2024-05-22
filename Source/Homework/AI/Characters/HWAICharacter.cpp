// Fill out your copyright notice in the Description page of Project Settings.


#include "HWAICharacter.h"


#include "BehaviorTree/BehaviorTree.h"
#include "Components/CharacterComponents/AIPatrollingComponent.h"

AHWAICharacter::AHWAICharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    AIPatrollingComponent = CreateDefaultSubobject<UAIPatrollingComponent>(TEXT("AIPatrolling"));
}

UAIPatrollingComponent* AHWAICharacter::GetAIPatrolling() const
{
    return AIPatrollingComponent;
}

UBehaviorTree* AHWAICharacter::GetBehaviorTree() const
{
    return BehaviorTree;
}
