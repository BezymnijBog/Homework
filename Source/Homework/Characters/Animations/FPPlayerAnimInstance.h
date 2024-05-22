// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HWBaseCharacterAnimInstance.h"
#include "FPPlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class HOMEWORK_API UFPPlayerAnimInstance : public UHWBaseCharacterAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animations | FirstPerson")
	float PlayerCameraPitchAngle = 0.f;

	TWeakObjectPtr<class AFPPlayerCharacter> CachedFPCharacterOwner;

private:
	float CalculateCameraPitchAngle() const;
};
