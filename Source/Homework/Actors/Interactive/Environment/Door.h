// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactive/Interface/Interactive.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "Subsystems/SaveSubsystem/SaveSubsystemInterface.h"

#include "Door.generated.h"

UCLASS()
class HOMEWORK_API ADoor : public AActor, public IInteractive, public ISaveSubsystemInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoor();

	virtual void Interact(AHWBaseCharacter* Character) override;

	virtual FName GetActionEventName() const override;

	virtual bool HasOnInteractionCallback() const override;

	virtual FDelegateHandle AddOnInteractionUFunction(UObject* Object, const FName& FunctionName) override;

	virtual void RemoveOnInteractionDelegate(FDelegateHandle DelegateHandle) override;
	
	virtual void Tick(float DeltaTime) override;

	virtual void OnLevelDeserialized_Implementation() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactive | Door")
	UStaticMeshComponent* DoorMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactive | Door")
	USceneComponent* DoorPivot;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactive | Door")
	UCurveFloat* DoorAnimCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive | Door")
	float AngleClosed = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive | Door")
	float AngleOpened = 90.f;

	FOnInteraction OnInteractionEvent;

private:
	UFUNCTION()
	void UpdateDoorAnimation(float Alpha);
	
	UFUNCTION()
	void OnDoorAnimationFinished();

	void InteractInternal();
	
	FTimeline DoorOpenTimeline;

	UPROPERTY(SaveGame)
	bool bIsOpened = false;
};
