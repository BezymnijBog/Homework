// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "HomeworkTypes.h"
#include "GameFramework/Pawn.h"
#include "Turret.generated.h"

class UNiagaraSystem;
class UPawnAttributesComponent;
class UWeaponBarellComponent;

UENUM()
enum class ETurretState : uint8
{
	Searching,
	RotateToStimulus,
	Firing,
	Dead
};

UCLASS()
class HOMEWORK_API ATurret : public APawn
{
	GENERATED_BODY()

public:
	ATurret();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PossessedBy(AController* NewController) override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Tick(float DeltaTime) override;
	void OnCurrentTargetSet();

	virtual FVector GetPawnViewLocation() const override;
	virtual FRotator GetViewRotation() const override;
	void RotateToStimulus(FVector StimulusLocation);

	UPROPERTY(ReplicatedUsing=OnRep_CurrentTarget)
	TWeakObjectPtr<AActor> CurrentTarget;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintNativeEvent)
	void OnDeath();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* TurretBaseComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* TurretBarellComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWeaponBarellComponent* WeaponBarell;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPawnAttributesComponent* TurretAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BaseFiringInterpSpeed = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BaseSearchingRotationRate = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BarellPitchRotationRate = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters", meta = (ClampMin = 0.f, UIMin = 0.f, ClampMax = 90.f, UIMax = 90.f))
	float MaxBarrelPitchAngle = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters", meta = (ClampMin = -90.f, UIMin = -90.f, ClampMax = 0.f, UIMax = 0.f))
	float MinBarrelPitchAngle = -20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters | Fire", meta = (ClampMin = 1.f, UIMin = 1.f))
	float RateOfFire = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters | Fire", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BulletSpreadAngle = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters | Fire", meta = (ClampMin = 0.f, UIMin = 0.f))
	float FireDelayTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters | Searching", meta = (ClampMin = 0.f, UIMin = 0.f))
	float StimulusSearchingTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters | Searching", meta = (ClampMin = 0.f, UIMin = 0.f))
	float StimulusSearchingInterpSpeed = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death | VFX")
	UParticleSystem* OnDeathVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters | Team")
	ETeams Team = ETeams::Enemy;

private:
	UFUNCTION()
	void OnRep_CurrentTarget();
	
	void MakeShot();
	void FiringMovement(float DeltaTime);
	void SearchingMovement(float DeltaTime);
	void StimulusRotation(float DeltaTime);
	void SetCurrentState(ETurretState NewState);

	float GetFireInterval() const;

	ETurretState CurrentState = ETurretState::Searching;
	FTimerHandle ShotTimer;
	FTimerHandle StimulusSearchTimer;
	FVector DirectionToLastStimulus = FVector::ZeroVector;
};
