// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../InteractiveActor.h"
#include "Ladder.generated.h"

class UBoxComponent;
/**
 * 
 */
UCLASS(Blueprintable)
class HOMEWORK_API ALadder : public AInteractiveActor
{
	GENERATED_BODY()

public:
	ALadder();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	float GetHeight() const { return LadderHeight; }
	bool IsOnTop() const { return bIsOnTop; }

	UBoxComponent* GetLadderInteractionBox() const;
	UAnimMontage* GetAttachFromTopAnimMontage() const;
	FVector GetAttachFromTopAnimMontageStartingLocation() const;
protected:
	virtual void OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder Paremeters")
	float LadderHeight = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder Paremeters")
	float LadderWidth = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder Paremeters")
	float StepsInterval = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder Paremeters")
	float BottomStepOffset = 25.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* RightRailMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* LeftRailMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UInstancedStaticMeshComponent* StepsMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* TopInteractionVolume;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder Parameters")
	UAnimMontage* AttachFromTopAnimMontage;

	// offset from ladder's top for starting anim montage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder Parameters")
	FVector AttachFromTopAnimMontageInitOffset = FVector::ZeroVector;

private:
	bool bIsOnTop = false;
};
