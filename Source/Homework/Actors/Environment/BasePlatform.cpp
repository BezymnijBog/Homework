// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlatform.h"
#include "Components/StaticMeshComponent.h"
#include "Actors/Environment/PlatformTrigger.h"

ABasePlatform::ABasePlatform()
{
	bReplicates = true;
    Super::SetReplicateMovement(true);
	PrimaryActorTick.bCanEverTick = true;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	SetRootComponent(PlatformMesh);
}

void ABasePlatform::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(MovementCurve))
	{
		FOnTimelineFloatStatic OnTimelineUpdate;
		OnTimelineUpdate.BindUFunction(this, FName("TickPlatformTimeline"));
		PlatformTimeline.AddInterpFloat(MovementCurve, OnTimelineUpdate);

		FOnTimelineEventStatic OnTimelineFinished;
		OnTimelineFinished.BindUFunction(this, FName("OnPlatformEndReached"));
		PlatformTimeline.SetTimelineFinishedFunc(OnTimelineFinished);
	}

	if (PlatformBehavior == EPlatformBehavior::Loop)
	{
		MovePlatform();
	}

	StartLocation = GetActorLocation();
	EndLocation = GetActorTransform().TransformPosition(EndPlatformLocation);

	if (IsValid(PlatformTrigger))
	{
		PlatformTrigger->OnTriggerActivated.AddDynamic(this, &ABasePlatform::OnPlatformTriggerActivated);
	}
}

void ABasePlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PlatformTimeline.TickTimeline(DeltaTime);
}

void ABasePlatform::MovePlatform()
{
	bIsMoving = true;
	if (bIsMovingForward)
	{
		PlatformTimeline.Reverse();
		bIsMovingForward = false;
	}
	else
	{
		PlatformTimeline.Play();
		bIsMovingForward = true;
	}
}

bool ABasePlatform::IsMoving() const
{
	return bIsMoving;
}

void ABasePlatform::OnPlatformTriggerActivated(bool bIsActivated)
{
	MovePlatform();
}

void ABasePlatform::TickPlatformTimeline(float Value)
{
	FVector NewLocation = FMath::Lerp(StartLocation, EndLocation, Value); 
	SetActorLocation(NewLocation);
}

void ABasePlatform::OnPlatformEndReached()
{
	bIsMoving = false;
	if (PlatformBehavior == EPlatformBehavior::Loop)
	{
		MovePlatform();
	}
	else if (ReturnTime > 0.0f )
	{
		GetWorld()->GetTimerManager().SetTimer(ReturnTimer, this, &ABasePlatform::MovePlatform, ReturnTime, false);
		return;
	}
}

