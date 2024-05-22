// Fill out your copyright notice in the Description page of Project Settings.


#include "WidgetPlayerAttributes.h"

ESlateVisibility UWidgetPlayerAttributes::IsParameterBarVisible(const float& ParameterPercent)
{
	return ParameterPercent < 1.f ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
}

ESlateVisibility UWidgetPlayerAttributes::IsHealthBarVisible() const
{
	return IsParameterBarVisible(HealthPercent);
}

ESlateVisibility UWidgetPlayerAttributes::IsStaminaBarVisible() const
{
	return IsParameterBarVisible(StaminaPercent);
}

ESlateVisibility UWidgetPlayerAttributes::IsOxygenBarVisible() const
{
	return IsParameterBarVisible(OxygenPercent);
}

void UWidgetPlayerAttributes::OnAttributesChanged(EAttributes Attribute, float AttributePercent)
{
	switch (Attribute)
	{
	case EAttributes::Health:
		HealthPercent = AttributePercent;
		break;
	case EAttributes::Oxygen:
		OxygenPercent = AttributePercent;
		break;
	case EAttributes::Stamina:
		StaminaPercent = AttributePercent;
		break;
	}
}
