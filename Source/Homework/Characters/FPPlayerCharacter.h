// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "FPPlayerCharacter.generated.h"

class AHWPlayerController;
/**
 * 
 */
UCLASS()
class HOMEWORK_API AFPPlayerCharacter : public APlayerCharacter
{
	GENERATED_BODY()

public:
	AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void Landed(const FHitResult& Hit) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	virtual FRotator GetViewRotation() const override;
	
protected:
	virtual void OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | First person")
	USkeletalMeshComponent* FirstPersonMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | First person")
	class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Ladder | Pitch", meta = (UIMin = -89.f, UIMax = 89.f))
	float LadderCameraPitchMin = -60.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Ladder | Pitch", meta = (UIMin = -89.f, UIMax = 89.f))
	float LadderCameraPitchMax = 80.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Ladder | Yaw", meta = (UIMin = -180.f, UIMax = 180.f))
	float LadderCameraYawMin = -90.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Ladder | Yaw", meta = (UIMin = -180.f, UIMax = 180.f))
	float LadderCameraYawMax = 90.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Zipline | Pitch", meta = (UIMin = -89.f, UIMax = 89.f))
	float ZiplineCameraPitchMin = -89.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Zipline | Pitch", meta = (UIMin = -89.f, UIMax = 89.f))
	float ZiplineCameraPitchMax = 89.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Zipline | Yaw", meta = (UIMin = -180.f, UIMax = 180.f))
	float ZiplineCameraYawMin = -90.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Zipline | Yaw", meta = (UIMin = -180.f, UIMax = 180.f))
	float ZiplineCameraYawMax = 90.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Hard landing")
	UAnimMontage* FPHardLandingMontage;

private:
	void SetCameraPitchBorders(float MinAngle, float MaxAngle);
	void SetCameraYawBorders(float MinAngle, float MaxAngle);
	void SetCameraDefaultAngles();
	
	FTimerHandle FPMontageTimer;

	TWeakObjectPtr<AHWPlayerController> HWPlayerController;

	void OnFPMontageTimerElapsed() const;
	bool IsMontagePlaying() const;
};
