// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HWBaseCharacter.h"
#include "Components/TimelineComponent.h"

#include "PlayerCharacter.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class HOMEWORK_API APlayerCharacter : public AHWBaseCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter(const FObjectInitializer& InObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void MoveForward(float InValue) override;
	virtual void MoveRight(float InValue) override;
	virtual void Turn(float InValue) override;
	virtual void LookUp(float InValue) override;
	virtual void TurnAtRate(float InValue) override;
	virtual void LookUpAtRate(float InValue) override;
	virtual void SwimForward(float InValue) override;
	virtual void SwimRight(float InValue) override;
	virtual void SwimUp(float InValue) override;

	virtual void OnStartSliding(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndSliding(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual void OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;
	virtual void OnSprintStart_Implementation() override;
	virtual void OnSprintStop_Implementation() override;
	
protected:
	virtual void OnStartAimInternal() override;
	virtual void OnStopAimInternal() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Camera")
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Camera")
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* SprintArmCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UCurveFloat* AimingCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint settings")
	float SprintArmLength = 400.f;
	
private:
	void AimingFOVUpdate(float InAlpha);
	void SprintArmUpdate(float InAlpha);

	FTimeline AimingTimeline;
	FTimeline SprintArmTimeline;

	float DefaultArmLength = 0.0f;	
};
