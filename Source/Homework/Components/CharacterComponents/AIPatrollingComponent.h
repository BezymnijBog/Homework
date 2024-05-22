// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIPatrollingComponent.generated.h"

class APatrollingPath;

UENUM(BlueprintType)
enum class EPatrollingType : uint8
{
	Circle,
	PingPong
};

USTRUCT(BlueprintType)
struct FPatrollingPathDescription
{
	GENERATED_BODY()
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	APatrollingPath* Path;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	EPatrollingType PatrollingType;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOMEWORK_API UAIPatrollingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	bool CanPatrol() const;

	FVector SelectClosestWayPoint();
	FVector SelectNextWayPoint();

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Path")
	FPatrollingPathDescription PathDescription;

private:
	void NextCircleIndex();
	void NextPingPongIndex();
	
	int32 CurrentWayPointIndex = -1;
	int32 PointIndexIncrement = 1;
};
