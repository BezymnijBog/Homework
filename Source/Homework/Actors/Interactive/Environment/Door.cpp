// Fill out your copyright notice in the Description page of Project Settings.


#include "Door.h"

#include "HomeworkTypes.h"

// Sets default values
ADoor::ADoor()
{
	USceneComponent* DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorRoot"));
	SetRootComponent(DefaultSceneRoot);

	DoorPivot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorPivot"));
	DoorPivot->SetupAttachment(GetRootComponent());

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(DoorPivot);
	
 	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ADoor::Interact(AHWBaseCharacter* Character)
{
	ensureMsgf(IsValid(DoorAnimCurve), TEXT("Door anim curve is not set"));
	InteractInternal();
	if (OnInteractionEvent.IsBound())
	{
		OnInteractionEvent.Broadcast();
	}
}

FName ADoor::GetActionEventName() const
{
	return ActionInteract;
}

bool ADoor::HasOnInteractionCallback() const
{
	return true;
}

FDelegateHandle ADoor::AddOnInteractionUFunction(UObject* Object, const FName& FunctionName)
{
	return OnInteractionEvent.AddUFunction(Object, FunctionName);
}

void ADoor::RemoveOnInteractionDelegate(FDelegateHandle DelegateHandle)
{
	OnInteractionEvent.Remove(DelegateHandle);
}

void ADoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	DoorOpenTimeline.TickTimeline(DeltaTime);

}

void ADoor::OnLevelDeserialized_Implementation()
{
	float YawAngle = bIsOpened ? AngleOpened : AngleClosed;
	DoorPivot->SetRelativeRotation(FRotator(0.f, YawAngle, 0.f));
}

void ADoor::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(DoorAnimCurve))
	{
		FOnTimelineFloatStatic DoorAnimationDelegate;
		DoorAnimationDelegate.BindUObject(this, &ADoor::UpdateDoorAnimation);
		DoorOpenTimeline.AddInterpFloat(DoorAnimCurve, DoorAnimationDelegate);

		FOnTimelineEventStatic DoorOpenedDelegate;
		DoorOpenedDelegate.BindUObject(this, &ADoor::OnDoorAnimationFinished);
		DoorOpenTimeline.SetTimelineFinishedFunc(DoorOpenedDelegate);
	}
}

void ADoor::UpdateDoorAnimation(float Alpha)
{
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);
	float YawAngle = FMath::Lerp(AngleClosed, AngleOpened, Alpha);
	DoorPivot->SetRelativeRotation(FRotator(0.f, YawAngle, 0.f));
}

void ADoor::OnDoorAnimationFinished()
{
	SetActorTickEnabled(false);
}

void ADoor::InteractInternal()
{
	SetActorTickEnabled(true);
	if (bIsOpened)
	{
		DoorOpenTimeline.Reverse();
	}
	else
	{
		DoorOpenTimeline.Play();
	}
	bIsOpened = !bIsOpened;
}

