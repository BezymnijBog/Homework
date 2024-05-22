// Fill out your copyright notice in the Description page of Project Settings.


#include "HWBaseCharacterAnimInstance.h"



#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Characters/HWBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Components/MovementComponents/HWBaseCharacterMovementComponent.h"

void UHWBaseCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<AHWBaseCharacter>(), TEXT("UHWBaseCharacterAnimInstance::NativeBeginPlay(): UHWBaseCharacterAnimInstance can be used only with AHWBaseCharacter"));
	CachedCharacter = StaticCast<AHWBaseCharacter*>(TryGetPawnOwner());
}

void UHWBaseCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!CachedCharacter.IsValid())
	{
		return;
	}

	bIsAiming = CachedCharacter->IsAiming();
	UHWBaseCharacterMovementComponent* CharacterMovement = CachedCharacter->GetBaseCharacterMovement();
	Speed = CharacterMovement->Velocity.Size();
	Direction = CalculateDirection(CharacterMovement->Velocity, CachedCharacter->GetActorRotation());
	bIsFlying = CharacterMovement->IsFalling();
	bIsCrouching = CharacterMovement->IsCrouching();
	bIsSprinting = CharacterMovement->IsSprinting();
	bIsOutOfStamina = CharacterMovement->IsOutOfStamina();
	bIsSwimming = CharacterMovement->IsSwimming();
	bIsProne = CharacterMovement->IsProne();
	bIsOnLadder = CharacterMovement->IsOnLadder();
	bIsOnZipline = CharacterMovement->IsOnZipline();
	bIsStrafing = !CharacterMovement->bOrientRotationToMovement;
	bIsWallRunning = CharacterMovement->IsWallRunning();
	WallRunSide = StaticCast<uint8>(CharacterMovement->GetWallRunSide());

	if (bIsOnLadder)
	{
		LadderSpeedRatio = CharacterMovement->GetLadderSpeedRatio();
	}

	if (!IsIKNeeded())
	{
		return;
	}
	const float BodyOffset = CachedCharacter->GetIKBodyOffset();
	const float RightFootOffset = CachedCharacter->GetIKRightFootOffset();
	// minus because left foot has different orientation
	const float LeftFootOffset = -CachedCharacter->GetIKLeftFootOffset();
	if (!bIsCrouching) 
	{
		RightFootEffectorLocation = FVector(RightFootOffset + BodyOffset, 0.f, 0.f);
		LeftFootEffectorLocation = FVector(LeftFootOffset - BodyOffset, 0.f, 0.f);
		IKBodyOffset = FVector(0.f, 0.f, BodyOffset);
	}
	else
	{
		RightFootEffectorLocation = FVector(RightFootOffset - BodyOffset, 0.f, 0.f);
		LeftFootEffectorLocation = FVector(LeftFootOffset + BodyOffset, 0.f, 0.f);
		IKBodyOffset = FVector(0.f, 0.f, BodyOffset);
	}

	AimRotator = CachedCharacter->GetAimOffset();
	
	const UCharacterEquipmentComponent* CharacterEquipment = CachedCharacter->GetEquipmentComponent();
	CurrentEquipedItemType = CharacterEquipment->GetCurrentEquippedItemType();
	
	ARangeWeaponItem* RangeWeapon = CharacterEquipment->GetCurrentRangeWeapon();
	if (IsValid(RangeWeapon))
	{
		ForeGripSocketTransform = RangeWeapon->GetForeGripTransform();
	}
}

bool UHWBaseCharacterAnimInstance::IsIKNeeded() const
{
	return !bIsFlying && !bIsSprinting && !bIsSwimming && !bIsOnLadder && !bIsOnZipline && !bIsProne;
}
