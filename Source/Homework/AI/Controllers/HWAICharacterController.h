// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Controllers/HWAIController.h"
#include "HWAICharacterController.generated.h"

class AHWAICharacter;

UCLASS()
class HOMEWORK_API AHWAICharacterController : public AHWAIController
{
	GENERATED_BODY()

public:
	virtual void ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors) override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
    virtual void SetPawn(APawn* InPawn) override;

protected:
	void SetupPatrolling();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float TargetReachRadius = 100.f;

private:
	void TryMoveToNextTarget();

	bool IsTargetReached(const FVector& TargetLocation) const;

	TWeakObjectPtr<AHWAICharacter> CachedCharacter;

	bool bIsPatrolling = false;
};
