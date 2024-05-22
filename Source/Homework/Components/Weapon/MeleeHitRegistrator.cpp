// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeHitRegistrator.h"

#include "DrawDebugHelpers.h"
#include "HomeworkTypes.h"
#include "HWTraceUtils/HWTraceUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/DebugSubsystem.h"

UMeleeHitRegistrator::UMeleeHitRegistrator()
{
	PrimaryComponentTick.bCanEverTick = true;
	SphereRadius = 5.f;
	UPrimitiveComponent::SetCollisionProfileName(CollisionProfileNoCollision);
}

void UMeleeHitRegistrator::ProcessHitRegistration()
{
	FVector CurrentLocation = GetComponentLocation();

	FHitResult HitResult;

#if ENABLE_DRAW_DEBUG
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	bool bIsDebugEnabled = DebugSubsystem->IsCategoryEnabled(DebugCategoryMeleeWeapon);
#elif
	bool bIsDebugEnabled = false;
#endif

	bool bHasHit = HWTraceUtils::SweepSphereSingleByChanel(GetWorld(), HitResult, PreviousComponentLocation, CurrentLocation, GetScaledSphereRadius(), ECC_Melee, FCollisionQueryParams::DefaultQueryParam, FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, 5.f);
	if (bHasHit)
	{
		FVector Direction = (CurrentLocation - PreviousComponentLocation).GetSafeNormal();
		if (OnMeleeHitRegistred.IsBound())
		{
			OnMeleeHitRegistred.Broadcast(HitResult, Direction);
		}
	}
}

void UMeleeHitRegistrator::SetHitRegistrationEnabled(bool bIsEnabled)
{
	bIsHitRegistrationEnabled = bIsEnabled;
}

void UMeleeHitRegistrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsHitRegistrationEnabled)
	{
		ProcessHitRegistration();
	}

	PreviousComponentLocation = GetComponentLocation();
}
