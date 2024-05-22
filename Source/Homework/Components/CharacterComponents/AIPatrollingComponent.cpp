// Fill out your copyright notice in the Description page of Project Settings.


#include "AIPatrollingComponent.h"

#include "Actors/Navigation/PatrollingPath.h"


bool UAIPatrollingComponent::CanPatrol() const
{
	return IsValid(PathDescription.Path) && PathDescription.Path->GetWayPoints().Num() > 0;
}

FVector UAIPatrollingComponent::SelectClosestWayPoint()
{
	FVector Result;
	FVector OwnerLocation = GetOwner()->GetActorLocation();
	const FTransform& PathTransform = PathDescription.Path->GetActorTransform();
	const TArray<FVector>& WayPoints = PathDescription.Path->GetWayPoints();
	float MinSquaredDistance = FLT_MAX;

	for (int32 i = 0; i < WayPoints.Num(); i++)
	{
		FVector WayPointWorld = PathTransform.TransformPosition(WayPoints[i]);
		float SquaredDistance = (OwnerLocation - WayPointWorld).SizeSquared();
		if (SquaredDistance < MinSquaredDistance)
		{
			MinSquaredDistance = SquaredDistance;
			Result = WayPointWorld;
			CurrentWayPointIndex = i;
		}
	}

	return Result;
}

FVector UAIPatrollingComponent::SelectNextWayPoint()
{
	switch (PathDescription.PatrollingType)
	{
	case EPatrollingType::Circle:
		NextCircleIndex();
		break;
	case EPatrollingType::PingPong:
		NextPingPongIndex();
		break;
	}
	const FTransform& PathTransform = PathDescription.Path->GetActorTransform();
	const FVector& WayPoint = PathDescription.Path->GetWayPoints()[CurrentWayPointIndex];
	return PathTransform.TransformPosition(WayPoint);
}

void UAIPatrollingComponent::NextCircleIndex()
{
	CurrentWayPointIndex += PointIndexIncrement;
	if (CurrentWayPointIndex == PathDescription.Path->GetWayPoints().Num())
	{
		CurrentWayPointIndex = 0;
	}
}

void UAIPatrollingComponent::NextPingPongIndex()
{
	CurrentWayPointIndex += PointIndexIncrement;
	if (CurrentWayPointIndex == PathDescription.Path->GetWayPoints().Num() - 1)
	{
		PointIndexIncrement = -1;
	}
	else if (CurrentWayPointIndex == 0)
	{
		PointIndexIncrement = 1;
	}
}
