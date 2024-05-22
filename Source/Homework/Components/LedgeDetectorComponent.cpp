// Fill out your copyright notice in the Description page of Project Settings.


#include "LedgeDetectorComponent.h"

#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Homework/HomeworkTypes.h"
#include "Homework/HWGameInstance.h"
#include "Homework/HWTraceUtils/HWTraceUtils.h"
#include "Homework/Subsystems/DebugSubsystem.h"
#include "Kismet/GameplayStatics.h"

bool ULedgeDetectorComponent::DetectLedge(FLedgeDescription& OutResult) const
{
	UCapsuleComponent* CapsuleComponent = CachedCharacterOwner->GetCapsuleComponent();

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.AddIgnoredActor(GetOwner());

#if ENABLE_DRAW_DEBUG
  	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	bool bIsDebugEnabled = DebugSubsystem->IsCategoryEnabled(DebugCategoryLedgeDetection);
#else
	bool bIsDebugEnabled = false;
#endif

	float DrawTime = 2.f;
	
	const float Offset = 2.f;
	FVector CharacterBottom = CachedCharacterOwner->GetActorLocation() - (CapsuleComponent->GetScaledCapsuleHalfHeight() - Offset) * FVector::UpVector;
	// 1. forward check
	float ForwardCheckCapsuleRadius = CapsuleComponent->GetScaledCapsuleRadius();
	float ForwardCheckCapsuleHalfHeight = (MaxLedgeHeight - MinLedgeHeight) * 0.5;
	
	FVector ForwardStartLocation = CharacterBottom + (MinLedgeHeight + ForwardCheckCapsuleHalfHeight) * FVector::UpVector;
	FVector ForwardEndLocation = ForwardStartLocation + CachedCharacterOwner->GetActorForwardVector() * ForwardCheckDistance;
	FHitResult ForwardHitResult;
	if (!HWTraceUtils::SweepCapsuleSingleByChanel(GetWorld(), ForwardHitResult, ForwardStartLocation, ForwardEndLocation, ForwardCheckCapsuleRadius, ForwardCheckCapsuleHalfHeight, FQuat::Identity, ECC_Climbing, QueryParams, FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, DrawTime))
	{
		return false;		
	}

	// 2. downward check	
	float DownwardCheckSphereRadius = CapsuleComponent->GetScaledCapsuleRadius();
	float DownwardDepthOffset = 10.f;
	FVector DownwardStartLocation = ForwardHitResult.ImpactPoint - ForwardHitResult.ImpactNormal * DownwardDepthOffset;
	DownwardStartLocation.Z = CharacterBottom.Z + MaxLedgeHeight + DownwardCheckSphereRadius;
	FVector DownwardEndLocation(DownwardStartLocation.X, DownwardStartLocation.Y, CharacterBottom.Z);
	FHitResult DownwardHitResult;

	if (!HWTraceUtils::SweepSphereSingleByChanel(GetWorld(), DownwardHitResult, DownwardStartLocation, DownwardEndLocation, DownwardCheckSphereRadius, ECC_Climbing, QueryParams, FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, DrawTime))
	{
		return false;
	}
	
	// 3. overlap check
	float OverlapCapsuleRadius = CapsuleComponent->GetScaledCapsuleRadius();
	float OverlapCapsuleHalfHeight = CachedCharacterOwner->GetDefaultHalfHeight();
	float FloorOffset = 2.f;
	FVector OverlapLocation = DownwardHitResult.ImpactPoint + (OverlapCapsuleHalfHeight + FloorOffset) * FVector::UpVector;

	if (HWTraceUtils::OverlapCapsuleBlockingByProfile(GetWorld(), OverlapLocation, OverlapCapsuleRadius, OverlapCapsuleHalfHeight, FQuat::Identity, CollisionProfilePawn, QueryParams, bIsDebugEnabled, DrawTime))
	{
		return false;
	}
	
	OutResult.LedgeComponent = ForwardHitResult.GetComponent();
	OutResult.Location = OverlapLocation;
	OutResult.Rotation = (ForwardHitResult.ImpactNormal * FVector(-1.f, -1.f, 0.f)).ToOrientationRotator();
	OutResult.LedgeNormal = ForwardHitResult.ImpactNormal;
	OutResult.LedgeBottom = DownwardHitResult.Location;
	return true;
}

// Called when the game starts
void ULedgeDetectorComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()->IsA<ACharacter>(), TEXT("ULedgeDetectorComponent::BeginPlay only a character can use ULedgeDetectorComponent"));
	CachedCharacterOwner = StaticCast<ACharacter*>(GetOwner());
}
