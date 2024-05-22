// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../InteractiveActor.h"
#include "Zipline.generated.h"

class UCapsuleComponent;
UCLASS(Blueprintable)
class HOMEWORK_API AZipline : public AInteractiveActor
{
	GENERATED_BODY()
	
public:
	AZipline();

	virtual void OnConstruction(const FTransform& Transform) override;

	FVector GetMoveDirection() const;
	FVector GetClosestCablePoint(const FVector& Location) const;

	bool IsDetachNeeded(const FVector& Location) const;

	UCapsuleComponent* GetInteractionCapsule() const;

protected:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline Paremeters")
	float PoleHeight = 250.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline Paremeters")
	float InteractionCapsuleRadius = 70.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline Paremeters")
	float DetachingDistance = 100.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
	UStaticMeshComponent* FirstPoleStaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
	UStaticMeshComponent* SecondPoleStaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
	UStaticMeshComponent* CableStaticMeshComponent;

private:
	UStaticMeshComponent* GetLowPole() const;
};
