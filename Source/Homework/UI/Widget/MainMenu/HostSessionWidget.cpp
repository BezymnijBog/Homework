// Fill out your copyright notice in the Description page of Project Settings.


#include "HostSessionWidget.h"

#include "HWGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UHostSessionWidget::CreateSession()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	check(GameInstance->IsA<UHWGameInstance>());
	UHWGameInstance* HWGameInstance = StaticCast<UHWGameInstance*>(GameInstance);

	HWGameInstance->LaunchLobby(4, ServerName, bIsLAN);
}
