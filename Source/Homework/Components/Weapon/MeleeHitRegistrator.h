// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "MeleeHitRegistrator.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMeleeHitRegistred, const FHitResult&, const FVector&);

UCLASS(meta = (BlueprintSpawnableComponent))
class HOMEWORK_API UMeleeHitRegistrator : public USphereComponent
{
	GENERATED_BODY()

public:
	UMeleeHitRegistrator();

	void ProcessHitRegistration();
	void SetHitRegistrationEnabled(bool bIsEnabled);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FOnMeleeHitRegistred OnMeleeHitRegistred;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee hit registration")
	bool bIsHitRegistrationEnabled = false;

private:
	FVector PreviousComponentLocation = FVector::ZeroVector;
};
