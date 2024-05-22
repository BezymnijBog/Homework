// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"


#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Camera/CameraComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Homework/Components/MovementComponents/HWBaseCharacterMovementComponent.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& InObjectInitializer) :
	Super(InObjectInitializer)
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring arm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = 1;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);

	Team = ETeams::Player;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	DefaultArmLength = SpringArmComponent->TargetArmLength;

	if (IsValid(SprintArmCurve))
	{
		FOnTimelineFloatStatic SprintArmTimelineUpdate;
		SprintArmTimelineUpdate.BindUObject(this, &APlayerCharacter::SprintArmUpdate);
		SprintArmTimeline.AddInterpFloat(SprintArmCurve, SprintArmTimelineUpdate);
	}
	if (IsValid(AimingCurve))
	{
		FOnTimelineFloatStatic AimingTimelineUpdate;
		AimingTimelineUpdate.BindUObject(this, &APlayerCharacter::AimingFOVUpdate);
		AimingTimeline.AddInterpFloat(AimingCurve, AimingTimelineUpdate);
	}
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AimingTimeline.TickTimeline(DeltaSeconds);
	SprintArmTimeline.TickTimeline(DeltaSeconds);
}

void APlayerCharacter::MoveForward(float InValue)
{
	Super::MoveForward(InValue);
	if ((GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling()) && !FMath::IsNearlyZero(InValue, 1.e-5f))
	{
		FRotator YawRotator(0.f, GetControlRotation().Yaw, 0.f);
		FVector ForwardVector = YawRotator.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardVector, InValue);
	}
}

void APlayerCharacter::MoveRight(float InValue)
{
	Super::MoveRight(InValue);
	if ((GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling()) && !FMath::IsNearlyZero(InValue, 1.e-5f))
	{
		FRotator YawRotator(0.f, GetControlRotation().Yaw, 0.f);
		FVector RightVector = YawRotator.RotateVector(FVector::RightVector);
		//AddMovementInput(GetActorRightVector(), InValue);
		AddMovementInput(RightVector, InValue);
	}
}

void APlayerCharacter::Turn(float InValue)
{
	const ARangeWeaponItem* CurrentRangeWeapon = GetEquipmentComponent()->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		InValue *= CurrentRangeWeapon->GetTurnModifier();
	}
	APawn::AddControllerYawInput(InValue);
}

void APlayerCharacter::LookUp(float InValue)
{
	const ARangeWeaponItem* CurrentRangeWeapon = GetEquipmentComponent()->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		InValue *= CurrentRangeWeapon->GetLookUpModifier();
	}
	APawn::AddControllerPitchInput(InValue);
}

void APlayerCharacter::TurnAtRate(float InValue)
{
	const ARangeWeaponItem* CurrentRangeWeapon = GetEquipmentComponent()->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		InValue *= CurrentRangeWeapon->GetTurnModifier();
	}
	APawn::AddControllerYawInput(InValue * BaseTurnRate * GetWorld()->GetDeltaSeconds());	
}

void APlayerCharacter::LookUpAtRate(float InValue)
{
	const ARangeWeaponItem* CurrentRangeWeapon = GetEquipmentComponent()->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		InValue *= CurrentRangeWeapon->GetLookUpModifier();
	}
	APawn::AddControllerPitchInput(InValue * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::SwimForward(float InValue)
{
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(InValue, 1.e-5f))
	{
		FRotator PitchYawRotator(GetControlRotation().Pitch, GetControlRotation().Yaw, 0.f);
		FVector ForwardVector = PitchYawRotator.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardVector, InValue);
	}
}

void APlayerCharacter::SwimRight(float InValue)
{
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(InValue, 1.e-5f))
	{
		FRotator YawRotator(0.f, GetControlRotation().Yaw, 0.f);
		FVector RightVector = YawRotator.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, InValue);
	}
}

void APlayerCharacter::SwimUp(float InValue)
{
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(InValue, 1.e-5f))
	{
		AddMovementInput(FVector::UpVector, InValue);
	}
}

void APlayerCharacter::OnStartSliding(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartSliding(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset += FVector(0.f, 0.f, HalfHeightAdjust);	
}

void APlayerCharacter::OnEndSliding(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndSliding(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset -= FVector(0.f, 0.f, HalfHeightAdjust);
}

void APlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset += FVector(0.f, 0.f, HalfHeightAdjust);
}

void APlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset -= FVector(0.f, 0.f, HalfHeightAdjust);
}

void APlayerCharacter::OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset += FVector(0.f, 0.f, HalfHeightAdjust);
}

void APlayerCharacter::OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset -= FVector(0.f, 0.f, HalfHeightAdjust);
}

bool APlayerCharacter::CanJumpInternal_Implementation() const
{
	return bIsCrouched && !GetBaseCharacterMovement()->IsMantling() || Super::CanJumpInternal_Implementation();
}

void APlayerCharacter::OnJumped_Implementation()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void APlayerCharacter::OnSprintStart_Implementation()
{
	Super::OnSprintStart_Implementation();
	SprintArmTimeline.Play();
}

void APlayerCharacter::OnSprintStop_Implementation()
{
	Super::OnSprintStop_Implementation();
	SprintArmTimeline.Reverse();
}

void APlayerCharacter::OnStartAimInternal()
{
	Super::OnStartAimInternal();
	APlayerController* PlayerController = GetController<APlayerController>();
	ARangeWeaponItem* CurrentRangeWeapon = GetEquipmentComponent()->GetCurrentRangeWeapon();
	if (IsValid(AimingCurve))
	{
		AimingTimeline.Play();
	}
	else if (IsValid(PlayerController) && IsValid(PlayerController->PlayerCameraManager) && IsValid(CurrentRangeWeapon))
	{
	    APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
		CameraManager->SetFOV(CurrentRangeWeapon->GetAimFOV());
	}
}

void APlayerCharacter::OnStopAimInternal()
{
	Super::OnStopAimInternal();
	APlayerController* PlayerController = GetController<APlayerController>();
	if (IsValid(AimingCurve))
	{
		AimingTimeline.Reverse();
	}
	else if (IsValid(PlayerController) && IsValid(PlayerController->PlayerCameraManager))
	{
		APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
		CameraManager->UnlockFOV();
	}
}

void APlayerCharacter::AimingFOVUpdate(float InAlpha)
{
	APlayerController* PlayerController = GetController<APlayerController>();
	ARangeWeaponItem* CurrentWeapon = GetEquipmentComponent()->GetCurrentRangeWeapon();
	if (IsValid(PlayerController) && IsValid(PlayerController->PlayerCameraManager) && IsValid(CurrentWeapon))
	{
		APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
		float DefaultFOV = CameraManager->DefaultFOV;
		float AimingFOV = CurrentWeapon->GetAimFOV();
		float InterpolatedFOV = FMath::Lerp(DefaultFOV, AimingFOV, InAlpha);
		CameraManager->SetFOV(InterpolatedFOV);
	}
}

void APlayerCharacter::SprintArmUpdate(float InAlpha)
{
	const float ArmLength = FMath::Lerp(DefaultArmLength, SprintArmLength, FMath::Min(1.f, InAlpha));
	SpringArmComponent->TargetArmLength = ArmLength;
}
