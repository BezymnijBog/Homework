// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "HomeworkTypes.h"
#include "Blueprint/UserWidget.h"
#include "ReticleWidget.generated.h"


class AEquipableItem;
UCLASS()
class HOMEWORK_API UReticleWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnAimingStateChanged(bool bIsAiming);

    UFUNCTION(BlueprintNativeEvent)
    void OnEquippedItemChanged(const AEquipableItem*EquippedItem);

    UFUNCTION(BlueprintNativeEvent)
    void OnDamageUpdated(float Percentage);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reticle")
	EReticleType CurrentReticle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reticle")
	float DamagePercentage = 0.f;

private:
	void SetupCurrentReticle();

	TWeakObjectPtr<const AEquipableItem> CurrentEquippedItem;
};
