// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Characters/HWBaseCharacter.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"
#include "HighlightInteractive.h"
#include "UI/Widget/AmmoWidget.h"
#include "UI/Widget/ReticleWidget.h"
#include "UI/Widget/WidgetPlayerAttributes.h"

void UPlayerHUDWidget::SetHighlightInteractiveVisibility(bool bIsVisible)
{
	if (!IsValid(InteractiveKey))
	{
		return;
	}
	if (bIsVisible)
	{
		InteractiveKey->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		InteractiveKey->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPlayerHUDWidget::SetHighlightInteractiveActionText(FName KeyName)
{
	if (IsValid(InteractiveKey))
	{
		InteractiveKey->SetActionText(KeyName);
	}
}

UReticleWidget* UPlayerHUDWidget::GetReticleWidget() const
{
	return WidgetTree->FindWidget<UReticleWidget>(ReticleWidgetName);
}

UAmmoWidget* UPlayerHUDWidget::GetAmmoWidget() const
{
	return WidgetTree->FindWidget<UAmmoWidget>(AmmoWidgetName);
}

UWidgetPlayerAttributes* UPlayerHUDWidget::GetAttributesWidget() const
{
	return WidgetTree->FindWidget<UWidgetPlayerAttributes>(WidgetAttributesName);
}

float UPlayerHUDWidget::GetHealthPercent() const
{
	float Result = 1.f;
	APawn* Pawn = GetOwningPlayerPawn();
	AHWBaseCharacter* Character = Cast<AHWBaseCharacter>(Pawn);
	if (IsValid(Character))
	{
		Result = Character->GetCharacterAttributes()->GetHealthPercent();
	}
	return Result;
}
