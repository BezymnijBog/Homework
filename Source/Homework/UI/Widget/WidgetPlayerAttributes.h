// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "HomeworkTypes.h"
#include "Blueprint/UserWidget.h"
#include "WidgetPlayerAttributes.generated.h"

/**
 * 
 */
UCLASS()
class HOMEWORK_API UWidgetPlayerAttributes : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes")
	float HealthPercent = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes")
	float StaminaPercent = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes")
	float OxygenPercent = 1.f;
	
private:
	static ESlateVisibility IsParameterBarVisible(const float& ParameterPercent);
	
	UFUNCTION(BlueprintCallable)
	ESlateVisibility IsHealthBarVisible() const;
	UFUNCTION(BlueprintCallable)
	ESlateVisibility IsStaminaBarVisible() const;
	UFUNCTION(BlueprintCallable)
	ESlateVisibility IsOxygenBarVisible() const;

	
	UFUNCTION()
	void OnAttributesChanged(EAttributes Attribute, float AttributePercent);
};
