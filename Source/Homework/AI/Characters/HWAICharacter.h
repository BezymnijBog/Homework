// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/HWBaseCharacter.h"
#include "HWAICharacter.generated.h"

class UBehaviorTree;
class UAIPatrollingComponent;

UCLASS(Blueprintable)
class HOMEWORK_API AHWAICharacter : public AHWBaseCharacter
{
	GENERATED_BODY()

public:
	AHWAICharacter(const FObjectInitializer& ObjectInitializer);

	UAIPatrollingComponent* GetAIPatrolling() const;
	UBehaviorTree* GetBehaviorTree() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAIPatrollingComponent* AIPatrollingComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	UBehaviorTree* BehaviorTree;
};
