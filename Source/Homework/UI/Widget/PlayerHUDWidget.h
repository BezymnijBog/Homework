// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class UHighlightInteractive;
class UWidgetPlayerAttributes;
class UAmmoWidget;
class UReticleWidget;
/**
 * 
 */
UCLASS()
class HOMEWORK_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHighlightInteractiveVisibility(bool bIsVisible);
	void SetHighlightInteractiveActionText(FName KeyName);
	
	UReticleWidget* GetReticleWidget() const;
	UAmmoWidget* GetAmmoWidget() const;
	UWidgetPlayerAttributes* GetAttributesWidget() const;

protected:
	UFUNCTION(BlueprintCallable)
	float GetHealthPercent() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget names")
	FName ReticleWidgetName = FName("Reticle");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget names")
	FName AmmoWidgetName = FName("Ammo");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget names")
	FName WidgetAttributesName = FName("Attributes");

	UPROPERTY(meta = (BindWidget))
	UHighlightInteractive* InteractiveKey;
};
