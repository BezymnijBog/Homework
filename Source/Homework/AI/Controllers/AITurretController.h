// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HWAIController.h"
#include "AITurretController.generated.h"

class ATurret;
/**
 * 
 */
UCLASS()
class HOMEWORK_API AAITurretController : public AHWAIController
{
	GENERATED_BODY()

public:
	virtual void ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors) override;
	virtual void SetPawn(APawn* InPawn) override;

private:
	bool SightUpdate();
	bool DamageUpdate();
	
	TWeakObjectPtr<ATurret> CachedTurret;
	
};
