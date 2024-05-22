// Fill out your copyright notice in the Description page of Project Settings.


#include "HWAttributeProgressBar.h"

#include "Components/ProgressBar.h"

void UHWAttributeProgressBar::SetProgressPercentage(float Percentage)
{
	if (IsValid(AttributeProgressBar))
	{
		AttributeProgressBar->SetPercent(Percentage);
	}
}
