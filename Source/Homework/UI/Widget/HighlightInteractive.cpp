// Fill out your copyright notice in the Description page of Project Settings.


#include "HighlightInteractive.h"

#include "Components/TextBlock.h"

void UHighlightInteractive::SetActionText(FName KeyName)
{
	if (IsValid(ActionText))
	{
		ActionText->SetText(FText::FromName(KeyName));
	}
}
