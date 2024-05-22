// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "HWAIController.generated.h"


class UAISense;
UCLASS()
class HOMEWORK_API AHWAIController : public AAIController
{
	GENERATED_BODY()

public:
	AHWAIController();

protected:
	AActor* GetClosestSensedActor(TSubclassOf<UAISense> SenseClass);
};
