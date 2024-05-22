// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactive.generated.h"

class AHWBaseCharacter;

UINTERFACE(MinimalAPI)
class UInteractive : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOMEWORK_API IInteractive
{
	GENERATED_BODY()
	
public:
	DECLARE_MULTICAST_DELEGATE(FOnInteraction);
	
	virtual void Interact(AHWBaseCharacter* Character) PURE_VIRTUAL(IInteractive::Interact, );
	virtual FName GetActionEventName() const PURE_VIRTUAL(IInteractive::GetActionEventName(), return FName(NAME_None););
	virtual bool HasOnInteractionCallback() const PURE_VIRTUAL(IInteractive::HasOnInteractionCallback, return false;);
	virtual FDelegateHandle AddOnInteractionUFunction(UObject* Object, const FName& FunctionName) PURE_VIRTUAL(IInteractive::AddOnInteractionUFunction, return FDelegateHandle(););
	virtual void RemoveOnInteractionDelegate(FDelegateHandle DelegateHandle) PURE_VIRTUAL(IInteractive::RemoveOnInteractionDelegate, );
};
