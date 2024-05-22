// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LedgeDetectorComponent.generated.h"


USTRUCT(BlueprintType)
struct FLedgeDescription
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ledge description")
	FVector Location;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ledge description")
	FRotator Rotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ledge description")
	FVector LedgeNormal;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ledge description")
	FVector LedgeBottom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ledge description")
	UPrimitiveComponent* LedgeComponent;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOMEWORK_API ULedgeDetectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	bool DetectLedge(OUT FLedgeDescription& OutResult) const;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Detection settings", meta = (UIMin = 0.0, ClampMin = 0.0))
	float MinLedgeHeight = 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Detection settings", meta = (UIMin = 0.0, ClampMin = 0.0))
	float MaxLedgeHeight = 200.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Detection settings", meta = (UIMin = 0.0, ClampMin = 0.0))
	float ForwardCheckDistance = 100.f;

	// Called when the game starts
	virtual void BeginPlay() override;

private:
	TWeakObjectPtr<ACharacter> CachedCharacterOwner;
};
