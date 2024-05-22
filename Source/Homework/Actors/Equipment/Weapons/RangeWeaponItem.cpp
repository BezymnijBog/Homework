// Fill out your copyright notice in the Description page of Project Settings.


#include "RangeWeaponItem.h"

#include "HomeworkTypes.h"

#include "Characters/HWBaseCharacter.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"
#include "Components/Weapon/WeaponBarellComponent.h"
#include "Net/UnrealNetwork.h"

ARangeWeaponItem::ARangeWeaponItem()
{
	SetReplicates(true);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);

	WeaponBarell = CreateDefaultSubobject<UWeaponBarellComponent>(TEXT("WeaponBarell"));
	WeaponBarell->SetupAttachment(WeaponMesh, SocketWeaponMuzzle);

	ReticleType = EReticleType::Default;

	EquippedSocketName = SocketCharacterWeapon;
}

void ARangeWeaponItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARangeWeaponItem, bIsReloading);
	DOREPLIFETIME(ARangeWeaponItem, bWasLstReloadSuccessful);
}

void ARangeWeaponItem::StartFire()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(ShotTimer))
	{
		return;
	}
	bIsFiring = true;
	MakeShot();
}

void ARangeWeaponItem::StopFire()
{
	bIsFiring = false;
}

void ARangeWeaponItem::StartAim()
{
	bIsAiming = true;
}

void ARangeWeaponItem::StopAim()
{
	bIsAiming = false;
}

void ARangeWeaponItem::StartReload()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_StartReload();
	}
	StartReloadInternal();
}

void ARangeWeaponItem::StartReloadInternal(bool bClientSimulation)
{
	if (bIsReloading && !bClientSimulation)
	{
		return;
	}

	AHWBaseCharacter* BaseCharacter = GetBaseCharacterOwner();
	bIsReloading = true;
	if (IsValid(BaseCharacter))
	{
		float MontageDuration = BaseCharacter->PlayAnimMontage(CharacterReloadMontage);
		PlayAnimMontage(WeaponReloadMontage);
		if (ReloadType == EReloadType::FullClip && HasAuthority())
		{
			auto OnReloadTimerElapsed = [this]() { EndReload(true); };
			GetWorld()->GetTimerManager().SetTimer(ReloadTimer, OnReloadTimerElapsed, FMath::Max(MontageDuration, 0.2f), false);
		}
	}
	else if (HasAuthority())
	{
		EndReload(true);
	}
}

void ARangeWeaponItem::EndReload(bool bIsSuccess)
{
	Server_EndReload(bIsSuccess);
}

void ARangeWeaponItem::EndReloadInternal(bool bIsSuccess, bool bClientSimulation)
{
	if (!bIsReloading && !bClientSimulation)
	{
		return;
	}
	AHWBaseCharacter* BaseCharacter = GetBaseCharacterOwner();
	if (!bIsSuccess && IsValid(BaseCharacter))
	{
		BaseCharacter->StopAnimMontage(CharacterReloadMontage);
		StopAnimMontage(WeaponReloadMontage);
	}

	if (ReloadType == EReloadType::ByBullet)
	{
		UAnimInstance* CharacterAnimInstance = IsValid(BaseCharacter) ? BaseCharacter->GetMesh()->GetAnimInstance() : nullptr;
		if (IsValid(CharacterAnimInstance))
		{
			CharacterAnimInstance->Montage_JumpToSection(SectionMontageReloadEnd, CharacterReloadMontage);
		}
		UAnimInstance* WeaponAnimInstance = WeaponMesh->GetAnimInstance();
		if (IsValid(WeaponAnimInstance))
		{
			WeaponAnimInstance->Montage_JumpToSection(SectionMontageReloadEnd, WeaponReloadMontage);
		}
	}
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);

	bIsReloading = false;
	if (bIsSuccess && OnReloadComplete.IsBound())
	{
		OnReloadComplete.Broadcast();
	}
}

void ARangeWeaponItem::SwitchBarellMode()
{
	if (WeaponBarell->SwitchBarellMode() && OnAmmoChanged.IsBound())
	{
		OnAmmoChanged.Broadcast(WeaponBarell->GetAmmo());
	}
}

void ARangeWeaponItem::RemoveAmmo(EAmunitionType AmunitionType)
{
	WeaponBarell->RemoveAmmo(AmunitionType);
}

float ARangeWeaponItem::GetAimFOV() const
{
	return AimFOV;
}

float ARangeWeaponItem::GetAimMaxMoveSpeed() const
{
	return AimMaxSpeed;
}

float ARangeWeaponItem::GetTurnModifier() const
{
	float Result = 1.f;
	if (bIsAiming)
	{
		Result = AimTurnModifier;
	}
	return Result;
}

float ARangeWeaponItem::GetLookUpModifier() const
{
	return bIsAiming ? AimLookUpModifier : 1.f;
}

int32 ARangeWeaponItem::GetAmmo() const
{
	return WeaponBarell->GetAmmo();
}

int32 ARangeWeaponItem::GetMaxAmmo() const
{
	return WeaponBarell->GetMaxAmmo();
}

EAmunitionType ARangeWeaponItem::GetAmmoType() const
{
	return WeaponBarell->GetAmmoType();
}

EReticleType ARangeWeaponItem::GetReticleType() const
{
	return bIsAiming ? AimReticleType : Super::GetReticleType();
}

FTransform ARangeWeaponItem::GetForeGripTransform() const
{
	return WeaponMesh->GetSocketTransform(SocketWeaponForeGrip);
}

void ARangeWeaponItem::SetAmmo(int32 NewAmmo)
{
	WeaponBarell->SetAmmo(NewAmmo);
	if (OnAmmoChanged.IsBound())
	{
		OnAmmoChanged.Broadcast(WeaponBarell->GetAmmo());
	}
}

bool ARangeWeaponItem::CanShoot() const
{
	return WeaponBarell->GetAmmo() > 0;
}

bool ARangeWeaponItem::IsAiming() const
{
	return bIsAiming;
}

bool ARangeWeaponItem::IsFiring() const
{
	return bIsFiring;
}

bool ARangeWeaponItem::IsReloading() const
{
	return bIsReloading;
}

float ARangeWeaponItem::GetCurrentBulletSpreadAngle() const
{
	float AngleDegrees = bIsAiming ? AimSpreadAngle : SpreadAngle;
	return FMath::DegreesToRadians(AngleDegrees);
}

float ARangeWeaponItem::GetShotTimerInterval() const
{
	return 60.f / WeaponBarell->GetROF();
}

void ARangeWeaponItem::TryAutoReload(AHWBaseCharacter* CharacterOwner)
{
	if (GetAmmo() == 0 && bAutoReload)
	{
		CharacterOwner->Reload();
	}
}

void ARangeWeaponItem::MakeShot()
{
	AHWBaseCharacter* CharacterOwner = GetBaseCharacterOwner();
	if (!IsValid(CharacterOwner))
	{
		return;
	}

	if (!CanShoot())
	{
		StopFire();
		TryAutoReload(CharacterOwner);
		return;
	}

	EndReload(false);
	CharacterOwner->PlayAnimMontage(CharacterFireMontage);
	PlayAnimMontage(WeaponFireMontage);

	FVector ShotLocation;
	FRotator ShotRotation;
	if (CharacterOwner->IsPlayerControlled())
	{
		APlayerController* PlayerController = CharacterOwner->GetController<APlayerController>();
		PlayerController->GetPlayerViewPoint(ShotLocation, ShotRotation);
	}
	else
	{
		ShotLocation = WeaponBarell->GetComponentLocation();
		ShotRotation = CharacterOwner->GetBaseAimRotation();
	}

	FVector ShotDirection = ShotRotation.RotateVector(FVector::ForwardVector);
	SetAmmo(GetAmmo() - 1);
	WeaponBarell->Shot(ShotLocation, ShotDirection, GetCurrentBulletSpreadAngle());
	CharacterOwner->GetCharacterInventory()->OnShot(GetAmmoType());
	TryAutoReload(CharacterOwner);

	GetWorld()->GetTimerManager().ClearTimer(ShotTimer);
	GetWorld()->GetTimerManager().SetTimer(ShotTimer, this, &ARangeWeaponItem::OnShotTimerElapsed, GetShotTimerInterval());
}

void ARangeWeaponItem::OnShotTimerElapsed()
{
	if (!bIsFiring)
	{
		return;
	}
	switch (WeaponBarell->GetFireMode())
	{
	case EWeaponFireMode::Single:
		StopFire();
		break;
	case EWeaponFireMode::FullAuto:
		MakeShot();
		break;
	}
}

float ARangeWeaponItem::PlayAnimMontage(UAnimMontage* AnimMontage) const
{
	UAnimInstance* WeaponAnimInstance = WeaponMesh->GetAnimInstance();
	float Result = 0.f;
	if (IsValid(WeaponAnimInstance))
	{
		Result = WeaponAnimInstance->Montage_Play(AnimMontage);
	}
	return Result;
}

void ARangeWeaponItem::StopAnimMontage(UAnimMontage* AnimMontage, float BlendOutTime)
{
	UAnimInstance* WeaponAnimInstance = WeaponMesh->GetAnimInstance();
	if (IsValid(WeaponAnimInstance))
	{
		WeaponAnimInstance->Montage_Stop(BlendOutTime, AnimMontage);
	}
}

void ARangeWeaponItem::OnRep_Reloading(bool bIsReloading_Old)
{
	if (bIsReloading && !bIsReloading_Old)
	{
		bIsReloading = bIsReloading_Old;
		StartReloadInternal(true);
	}
	else if (!bIsReloading && bIsReloading_Old)
	{
		EndReloadInternal(bWasLstReloadSuccessful, true);
	}
}

void ARangeWeaponItem::Server_StartReload_Implementation()
{
	StartReloadInternal();
}

void ARangeWeaponItem::Server_EndReload_Implementation(bool bIsSuccess)
{
	EndReloadInternal(bIsSuccess);
	bWasLstReloadSuccessful = bIsSuccess;
}
