// Fill out your copyright notice in the Description page of Project Settings.


#include "JoinSessionWidget.h"

#include "HWGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UJoinSessionWidget::CloseWidget()
{
	HWGameInstance->OnMatchFound.RemoveAll(this);
	Super::CloseWidget();
}

void UJoinSessionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	check(GameInstance->IsA<UHWGameInstance>());
	HWGameInstance = StaticCast<UHWGameInstance*>(GameInstance);
}

void UJoinSessionWidget::FindOnlineSession()
{
	HWGameInstance->OnMatchFound.AddDynamic(this, &UJoinSessionWidget::OnMatchFound);
	HWGameInstance->FindMatch(bIsLAN);
	SearchingSessionState = ESearchingSessionState::Searching;
}

void UJoinSessionWidget::JoinOnlineSession()
{
	HWGameInstance->JoinOnlineGame();
}

void UJoinSessionWidget::OnMatchFound_Implementation(bool bIsSuccess)
{
	SearchingSessionState = bIsSuccess ? ESearchingSessionState::FoundSession : ESearchingSessionState::None;
	HWGameInstance->OnMatchFound.RemoveAll(this);
}
