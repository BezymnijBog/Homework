// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformInvocator.h"

// Sets default values
APlatformInvocator::APlatformInvocator()
{
}

void APlatformInvocator::Invoke() const
{
	if (OnInvocatorActivated.IsBound())
	{
		OnInvocatorActivated.Broadcast();
	}
}

