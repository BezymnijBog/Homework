// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Components/TimelineComponent.h"

#include "Bow.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDamageUpdated, float, Percentage);

/**
 * 
 */
UCLASS()
class HOMEWORK_API ABow : public ARangeWeaponItem
{
	GENERATED_BODY()

public:
	ABow();

	virtual void Tick(float DeltaSeconds) override;
	
	virtual void StartAim() override;
	virtual void StopAim() override;

	void Unload();
	
	virtual bool CanShoot() const override;

	mutable FOnDamageUpdated OnDamageUpdated;
	
protected:
	virtual void BeginPlay() override;

	virtual void MakeShot() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aiming")
	UCurveVector* ShotDataCurve;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aiming")
	UCurveFloat* ArrowDamageCurve;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aiming")
	UCurveFloat* CameraShakeCurve;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aiming")
	UAnimMontage* OverdrawMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aiming")
	float MinArrowSpeed = 1000.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aiming")
	float MaxArrowSpeed = 5000.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aiming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MinArrowDamage = 20.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aiming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxArrowDamage = 100.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aiming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float NormalHoldArrowTime = 8.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aiming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxArrowHoldTime = 10.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aiming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MinShakeAngle = 0.1f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aiming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxShakeAngle = 0.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aiming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ShakeFrequency = 10.f;

private:
	virtual float GetCurrentBulletSpreadAngle() const override;

	float GetShakeTimeInterval() const;
	
	void InterpolateShotParams(float DeltaSeconds);
	
	void InterpolateArrowDamage(float DeltaSeconds);
	
	void InterpolateShakeAngle(float DeltaSeconds);

	void ProcessCameraShake(float DeltaTime) const;
	
	void ResetParameters();
	
	void StartCameraShake();
	
	void UnloadInternal();

	void UpdateArrowDamage(float DeltaSeconds);

	void UpdateShotParams(float DeltaSeconds);

	void UpdateShakeAngle(float DeltaSeconds);
	
	UFUNCTION()
	void OnStringIsStretched();
	
	UFUNCTION()
	void OnStringIsFullyStretched();

	UFUNCTION()
	void UpdateShakeAngleWithTimeline(float Alpha);

	UFUNCTION()
	void UpdateArrowDamageWithTimeline(float Alpha);
	
	UFUNCTION()
	void UpdateShotParamsWithTimeline(FVector AlphaVector);

	UFUNCTION(Server, Reliable)
	void Server_Unload();
	
	FTimeline StringStretchTimeline;
	FTimeline CameraShakeTimeline;
	FTimeline ArrowDamageTimeline;
	FTimerHandle BowShotTimer;

	float CurrentSpreadAngle = SpreadAngle;
	float CurrentArrowSpeed = MinArrowSpeed;
	float CurrentArrowDamage = MinArrowDamage;
	float CurrentShakeAngle = MinShakeAngle;
	
	float InterpolationTime = 1.f;

	bool bApplyCameraShake = false;
	bool bStringIsStretched = false;
	bool bStringIsFullyStretched = false;
};
