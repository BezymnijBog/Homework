// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_ChangeMovementMode.generated.h"

/**
 * 
 */
UCLASS()
class HOMEWORK_API UAnimNotify_ChangeMovementMode : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim Notify")
	TEnumAsByte<EMovementMode> NewMovementMode = EMovementMode::MOVE_Walking;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim Notify", meta = (EditCondition = "NewMovementMode == EMovementMode::MOVE_Custom"))
	uint8 CustomMovementMode = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim Notify")
	bool bUseControllerRotationYaw = false;
};
