// Fill out your copyright notice in the Description page of Project Settings.


#include "FPPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Controllers/HWPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Homework/HomeworkTypes.h"

AFPPlayerCharacter::AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	FirstPersonMeshComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -86.f));
	FirstPersonMeshComponent->CastShadow = false;
	FirstPersonMeshComponent->bCastDynamicShadow = false;
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);
	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent, SocketFPCamera);
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bCastHiddenShadow = true;

	CameraComponent->bAutoActivate = false;

	SpringArmComponent->bAutoActivate = false;
	SpringArmComponent->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	
	bUseControllerRotationYaw = true;
}

void AFPPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	float FallHeight = (CurrentFallApex - GetActorLocation()).Z;
	UAnimInstance* FPAnimInstance = FirstPersonMeshComponent->GetAnimInstance();
	if (FallHeight > HardLandingHeight && IsValid(FPHardLandingMontage) && IsValid(FPAnimInstance))
	{
		FPAnimInstance->Montage_Play(FPHardLandingMontage);
	}
}

void AFPPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (IsMontagePlaying() && HWPlayerController.IsValid())
	{
		FRotator TargetControlRotation = HWPlayerController->GetControlRotation();
		TargetControlRotation.Pitch = 0.f;
		float BlendSpeed = 300.f;
		TargetControlRotation = FMath::RInterpTo(HWPlayerController->GetControlRotation(), TargetControlRotation, DeltaSeconds, BlendSpeed);
		HWPlayerController->SetControlRotation(TargetControlRotation);
	}
}

void AFPPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	HWPlayerController = Cast<AHWPlayerController>(NewController);	
}

void AFPPlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	const AFPPlayerCharacter* DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());
	FVector& FPMeshRelativeLocation = FirstPersonMeshComponent->GetRelativeLocation_DirectMutable();
	FPMeshRelativeLocation.Z = DefaultCharacter->FirstPersonMeshComponent->GetRelativeLocation().Z + HalfHeightAdjust;
}

void AFPPlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	const AFPPlayerCharacter* DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());
	FVector& FPMeshRelativeLocation = FirstPersonMeshComponent->GetRelativeLocation_DirectMutable();
	FPMeshRelativeLocation.Z = DefaultCharacter->FirstPersonMeshComponent->GetRelativeLocation().Z;
}

void AFPPlayerCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	if (!HWPlayerController.IsValid())
	{
		return;
	}

	const ECustomMovementMode PrevCustomMovementMode = StaticCast<ECustomMovementMode>(PreviousCustomMode);
	
	if (GetBaseCharacterMovement()->IsOnLadder())
	{
		HWPlayerController->SetIgnoreCameraPitch(true);
		bUseControllerRotationYaw = false;
		SetCameraPitchBorders(LadderCameraPitchMin, LadderCameraPitchMax);
		SetCameraYawBorders(LadderCameraYawMin, LadderCameraYawMax);
	}
	else if (PrevCustomMovementMode == ECustomMovementMode::CMOVE_Ladder)
	{
		HWPlayerController->SetIgnoreCameraPitch(true);
		bUseControllerRotationYaw = true;
		SetCameraDefaultAngles();
	}
	else if (GetBaseCharacterMovement()->IsOnZipline())
	{
		HWPlayerController->SetIgnoreCameraPitch(true);
		bUseControllerRotationYaw = false;
		SetCameraPitchBorders(ZiplineCameraPitchMin, ZiplineCameraPitchMax);
		SetCameraYawBorders(ZiplineCameraYawMin, ZiplineCameraYawMax);
	}
	else if (PrevCustomMovementMode == ECustomMovementMode::CMOVE_Zipline)
	{
		HWPlayerController->SetIgnoreCameraPitch(true);
		bUseControllerRotationYaw = true;
		SetCameraDefaultAngles();
	}
}

FRotator AFPPlayerCharacter::GetViewRotation() const
{
	FRotator Result = Super::GetViewRotation();
	if (IsMontagePlaying())
	{
		FRotator SocketRotation= FirstPersonMeshComponent->GetSocketRotation(SocketFPCamera);
		Result.Yaw = SocketRotation.Yaw;
		Result.Roll = SocketRotation.Roll;
		Result.Pitch += SocketRotation.Pitch;
	}
	return Result;
}

void AFPPlayerCharacter::OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime)
{
	Super::OnMantle(MantlingSettings, MantlingAnimationStartTime);
	UAnimInstance* FPAnimInstance = FirstPersonMeshComponent->GetAnimInstance();
	if (IsValid(FPAnimInstance) && IsValid(MantlingSettings.FPMantlingMontage))
	{
		if (HWPlayerController.IsValid())
		{
			HWPlayerController->SetIgnoreLookInput(true);
			HWPlayerController->SetIgnoreMoveInput(true);
		}
		float MontageDuration = FPAnimInstance->Montage_Play(MantlingSettings.FPMantlingMontage, 1, EMontagePlayReturnType::Duration, MantlingAnimationStartTime);
		GetWorld()->GetTimerManager().SetTimer(FPMontageTimer, this, &AFPPlayerCharacter::OnFPMontageTimerElapsed, MontageDuration);
	}
}

void AFPPlayerCharacter::SetCameraPitchBorders(float MinAngle, float MaxAngle)
{
	APlayerCameraManager* CameraManager = HWPlayerController->PlayerCameraManager;
	CameraManager->ViewPitchMin = GetActorForwardVector().ToOrientationRotator().Pitch + MinAngle;
	CameraManager->ViewPitchMax = GetActorForwardVector().ToOrientationRotator().Pitch + MaxAngle;
}

void AFPPlayerCharacter::SetCameraYawBorders(float MinAngle, float MaxAngle)
{
	APlayerCameraManager* CameraManager = HWPlayerController->PlayerCameraManager;
	CameraManager->ViewYawMin = GetActorForwardVector().ToOrientationRotator().Yaw + MinAngle;
	CameraManager->ViewYawMax = GetActorForwardVector().ToOrientationRotator().Yaw + MaxAngle;
}

void AFPPlayerCharacter::SetCameraDefaultAngles()
{
	APlayerCameraManager* CameraManager = HWPlayerController->PlayerCameraManager;
	APlayerCameraManager* DefaultCameraManager = CameraManager->GetClass()->GetDefaultObject<APlayerCameraManager>();
	SetCameraPitchBorders(DefaultCameraManager->ViewPitchMin, DefaultCameraManager->ViewPitchMax);
	SetCameraYawBorders(DefaultCameraManager->ViewYawMin, DefaultCameraManager->ViewYawMax);
}

void AFPPlayerCharacter::OnFPMontageTimerElapsed() const
{
	if (HWPlayerController.IsValid())
	{
		HWPlayerController->SetIgnoreLookInput(false);
		HWPlayerController->SetIgnoreMoveInput(false);
	}
}

bool AFPPlayerCharacter::IsMontagePlaying() const
{
	UAnimInstance* AnimInstance = FirstPersonMeshComponent->GetAnimInstance();
	return IsValid(AnimInstance) && AnimInstance->IsAnyMontagePlaying();
}
