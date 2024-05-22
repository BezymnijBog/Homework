// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacterSpawner.h"


#include "Actors/Interactive/Interface/Interactive.h"
#include "AI/Characters/HWAICharacter.h"

// Sets default values
AAICharacterSpawner::AAICharacterSpawner()
{
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnRoot"));
	SetRootComponent(SceneRoot);
}

void AAICharacterSpawner::SpawnAI()
{
	if (!bCanSpawn || !IsValid(CharacterClass))
	{
		return;
	}

	AHWAICharacter* AICharacter = GetWorld()->SpawnActor<AHWAICharacter>(CharacterClass, GetTransform());
	if (!IsValid(AICharacter->Controller))
	{
		AICharacter->SpawnDefaultController();
	}

	if (bDoOnce)
	{
		UnSubscribeFromTrigger();
		bCanSpawn = false;
	}
}

// Called when the game starts or when spawned
void AAICharacterSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (SpawnTrigger.GetInterface())
	{
		SpawnTrigger->AddOnInteractionUFunction(this, FName("SpawnAI"));
	}
	
	if (bIsSpawnOnStart)
	{
		SpawnAI();
	}
}

void AAICharacterSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnSubscribeFromTrigger();
	Super::EndPlay(EndPlayReason);
}

#if WITH_EDITOR
void AAICharacterSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property->GetName() == GET_MEMBER_NAME_STRING_CHECKED(AAICharacterSpawner, SpawnTriggerActor))
	{
		SpawnTrigger = SpawnTriggerActor;
		if (!SpawnTrigger.GetInterface() || !SpawnTrigger->HasOnInteractionCallback())
		{
			SpawnTriggerActor = nullptr;
			SpawnTrigger = nullptr;
		}
	}
}
#endif

void AAICharacterSpawner::UnSubscribeFromTrigger()
{
	if (TriggerHandle.IsValid() && SpawnTrigger.GetInterface())
	{
		SpawnTrigger->RemoveOnInteractionDelegate(TriggerHandle);
	}
}

