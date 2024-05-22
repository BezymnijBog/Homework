// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugSubsystem.h"

bool UDebugSubsystem::IsCategoryEnabled(const FName& CategoryName) const
{
	const bool* bIsEnabled = EnabledDebugCatecories.Find(CategoryName);
	return bIsEnabled && *bIsEnabled; 
}
void UDebugSubsystem::EnableCategory(const FName& CategoryName, bool bIsEnabled)
{
	EnabledDebugCatecories.FindOrAdd(CategoryName);
	EnabledDebugCatecories[CategoryName] = bIsEnabled;
}