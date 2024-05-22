// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/MainMenu/NetworkWidget.h"
#include "JoinSessionWidget.generated.h"

class UHWGameInstance;
UENUM(BlueprintType)
enum class ESearchingSessionState : uint8
{
	None,
	Searching,
	FoundSession
};

UCLASS()
class HOMEWORK_API UJoinSessionWidget : public UNetworkWidget
{
	GENERATED_BODY()

protected:
	virtual void CloseWidget() override;
	
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void FindOnlineSession();

	UFUNCTION(BlueprintCallable)
	void JoinOnlineSession();

	UFUNCTION(BlueprintNativeEvent)
	void OnMatchFound(bool bIsSuccess);
	
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Network session")
	ESearchingSessionState SearchingSessionState = ESearchingSessionState::None;

private:
	TWeakObjectPtr<UHWGameInstance> HWGameInstance;
};
