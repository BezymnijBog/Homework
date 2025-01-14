// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HWBasePawnAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class HOMEWORK_API UHWBasePawnAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Base pawn animation instance")
	float InputForward;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Base pawn animation instance")
	float InputRight;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Base pawn animation instance")
	bool bIsInAir;

private:
	TWeakObjectPtr<class AHomeworkBasePawn> CachedBasePawn;
};
