// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkWidget.h"

FText UNetworkWidget::GetNetworkType() const
{
	return bIsLAN ? FText::FromString("LAN") : FText::FromString("Internet");
}

void UNetworkWidget::ToggleNetworkType()
{
	bIsLAN = !bIsLAN;
}

void UNetworkWidget::CloseWidget()
{
	if (OnNetworkWidgetClosed.IsBound())
	{
		OnNetworkWidgetClosed.Broadcast();
	}

	SetVisibility(ESlateVisibility::Hidden);
}
