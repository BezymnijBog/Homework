// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBarellComponent.h"

#include "DrawDebugHelpers.h"
#include "HomeworkTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/DebugSubsystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/DecalComponent.h"
#include "Net/UnrealNetwork.h"

UWeaponBarellComponent::UWeaponBarellComponent()
{
	SetIsReplicatedByDefault(true);
}

void UWeaponBarellComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	FDoRepLifetimeParams RepParams;
	RepParams.Condition = COND_SimulatedOnly;
	RepParams.RepNotifyCondition = REPNOTIFY_Always;
	DOREPLIFETIME_WITH_PARAMS(UWeaponBarellComponent, LastShotsInfo, RepParams);
	DOREPLIFETIME_WITH_PARAMS(UWeaponBarellComponent, ProjectilePool, RepParams);
	DOREPLIFETIME_WITH_PARAMS(UWeaponBarellComponent, CurrentProjectileIndex, RepParams);
	DOREPLIFETIME_WITH_PARAMS(UWeaponBarellComponent, Ammo, RepParams);
	DOREPLIFETIME_WITH_PARAMS(UWeaponBarellComponent, bUseCustomFireDamage, RepParams);
	DOREPLIFETIME_WITH_PARAMS(UWeaponBarellComponent, CustomFireDamage, RepParams);
}

void UWeaponBarellComponent::Shot(const FVector& ShotStart, FVector ShotDirection, float SpreadAngle)
{
	TArray<FShotInfo> ShotsInfo;
	for (int i = 0; i < GetActiveMode().BulletsPerShot; i++)
	{
		ShotDirection += GetBulletSpreadOfset(FMath::RandRange(0.f, SpreadAngle), ShotDirection.ToOrientationRotator());
		ShotDirection = ShotDirection.GetSafeNormal();
		ShotsInfo.Emplace(ShotStart, ShotDirection);
		if (bUseCustomFireDamage)
		{
			ShotsInfo.Last().SetCustomShotDamage(CustomFireDamage);
		}
		if (ProjectileInitialSpeed > 0.f)
		{
			ShotsInfo.Last().SetCustomProjectileSpeed(ProjectileInitialSpeed);
		}
	}
	
	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_Shot(ShotsInfo);
	}
	ShotInternal(ShotsInfo);
	SetProjectileSpeed(-1.f);
}

void UWeaponBarellComponent::SetAmmo(int32 NewAmmo)
{
	Ammo = NewAmmo;
}

void UWeaponBarellComponent::SetDamage(float Damage)
{
	CustomFireDamage = Damage;
	bUseCustomFireDamage = true;
}

void UWeaponBarellComponent::SetProjectileSpeed(float Speed)
{
	ProjectileInitialSpeed = Speed;
}

bool UWeaponBarellComponent::SwitchBarellMode()
{
	if (BarellModes.Num() == 1)
	{
		return false;

	}
	GetActiveMode().Ammo = Ammo;
	ActiveMode++;
	if (ActiveMode >= BarellModes.Num())
	{
		ActiveMode = 0;
	}
	Ammo = FMath::Min(GetActiveMode().Ammo, GetActiveMode().MaxAmmo);
	ReserveProjectilePool();
	return true;
}

int32 UWeaponBarellComponent::GetAmmo() const
{
	return Ammo;
}

int32 UWeaponBarellComponent::GetMaxAmmo() const
{
	return GetActiveMode().MaxAmmo;
}

float UWeaponBarellComponent::GetROF() const
{
	return GetActiveMode().RateOfFire;
}

float UWeaponBarellComponent::GetDamage() const
{
	return bUseCustomFireDamage ? CustomFireDamage : GetActiveMode().BaseFireDamage;
}

EAmunitionType UWeaponBarellComponent::GetAmmoType() const
{
	return GetActiveMode().AmmoType;
}

EWeaponFireMode UWeaponBarellComponent::GetFireMode() const
{
	return GetActiveMode().FireMode;
}

void UWeaponBarellComponent::ReserveProjectilePool()
{
	while (ProjectilePool.Num() > 0)
	{
		if (IsValid(ProjectilePool.Last()))
		{
			ProjectilePool.Last()->Destroy();
		}
		ProjectilePool.Pop();
	}

	if (GetOwnerRole() == ROLE_Authority && IsValid(GetActiveMode().ProjectileClass))
	{
		ProjectilePool.Reserve(GetActiveMode().ProjectilePoolSize);
		for (int32 i = 0; i < GetActiveMode().ProjectilePoolSize; ++i)
		{
			AHWProjectile* Projectile = GetWorld()->SpawnActor<AHWProjectile>(GetActiveMode().ProjectileClass, ProjectilePoolLocation, FRotator::ZeroRotator);
			Projectile->SetOwner(GetOwningPawn());
			Projectile->SetProjectileActive(false);
			ProjectilePool.Add(Projectile);
		}
	}
}

void UWeaponBarellComponent::RemoveAmmo(EAmunitionType AmunitionType)
{
	for (FWeaponBarellDescription& BarellMode : BarellModes)
	{
		if (BarellMode.AmmoType == AmunitionType)
		{
			BarellMode.Ammo = 0;
		}
	}
	Ammo = GetActiveMode().Ammo;
}

void UWeaponBarellComponent::BeginPlay()
{
	Super::BeginPlay();

	Ammo = GetActiveMode().Ammo;
	ReserveProjectilePool();
}

FWeaponBarellDescription& UWeaponBarellComponent::GetActiveMode()
{
	return BarellModes[ActiveMode];
}

const FWeaponBarellDescription& UWeaponBarellComponent::GetActiveMode() const
{
	return BarellModes[ActiveMode];
}

void UWeaponBarellComponent::ShotInternal(const TArray<FShotInfo>& ShotsInfo)
{
	if (GetOwner()->HasAuthority())
	{
		LastShotsInfo = ShotsInfo;
		if (!GetOwningPawn()->IsLocallyControlled())
		{
			Ammo--;
		}
	}

	const FVector MuzzleLocation = GetComponentLocation();
	UNiagaraComponent* TraceFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), GetActiveMode().TraceFX, MuzzleLocation, GetComponentRotation());
	for (const FShotInfo& ShotInfo : ShotsInfo)
	{
		FVector ShotDirection = ShotInfo.GetDirection();
		FVector ShotStart = ShotInfo.GetLocation();
		FVector ShotEnd = ShotStart + GetActiveMode().FireRange * ShotDirection;

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), GetActiveMode().MuzzleFlashFX, MuzzleLocation, GetComponentRotation());

#if ENABLE_DRAW_DEBUG
		UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
		bool bIsDebugEnabled = DebugSubsystem->IsCategoryEnabled(DebugCategoryRangeWeapon);
#elif
		bool bIsDebugEnabled = false;
#endif

		bool bHasHit = false;
		switch (GetActiveMode().HitRegistration)
		{
		case EHitRegistrationType::HitScan:
			bHasHit = HitScan(ShotStart, ShotEnd, ShotDirection);
			if (bIsDebugEnabled && bHasHit)
			{
				DrawDebugSphere(GetWorld(), ShotEnd, 10.f, 24, FColor::Red, false, 1.f);
			}
			break;
		case EHitRegistrationType::Projectile:
			LaunchProjectile(ShotInfo);
			break;
		}
		if (IsValid(TraceFXComponent))
		{
			TraceFXComponent->SetVectorParameter(FXParamTraceEnd, ShotEnd);
		}
		if (bIsDebugEnabled)
		{
			DrawDebugLine(GetWorld(), MuzzleLocation, ShotEnd, FColor::Red, false, 1.f, 0, 3.f);
		}

	}
}

void UWeaponBarellComponent::Server_Shot_Implementation(const TArray<FShotInfo>& ShotsInfo)
{
	ShotInternal(ShotsInfo);
}

void UWeaponBarellComponent::OnRep_LastShotsInfo()
{
	ShotInternal(LastShotsInfo);
}

void UWeaponBarellComponent::ProcessHit(const FHitResult& HitResult, const FVector& Direction)
{
	AActor* HitActor = HitResult.GetActor();
	if (GetOwner()->HasAuthority() && IsValid(HitActor))
	{
		float FireDamage = GetDamage();
		if (IsValid(GetActiveMode().FalloffDiagram))
		{
			float RelativeHitDistance = (HitResult.ImpactPoint - GetComponentLocation()).Size() / GetActiveMode().FireRange;
			float DamageFactor = GetActiveMode().FalloffDiagram->GetFloatValue(RelativeHitDistance);
			FireDamage *= DamageFactor;
		}
		FPointDamageEvent DamageEvent;
		DamageEvent.HitInfo = HitResult;
		DamageEvent.ShotDirection = Direction;
		DamageEvent.DamageTypeClass = GetActiveMode().DamageTypeClass;
		HitActor->TakeDamage(FireDamage, DamageEvent, GetController(), GetOwner());
	}
	UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), GetActiveMode().DefaultDecalInfo.DecalMaterial, GetActiveMode().DefaultDecalInfo.DecalSize, HitResult.ImpactPoint, HitResult.ImpactNormal.ToOrientationRotator());
	if (IsValid(DecalComponent))
	{
		DecalComponent->SetFadeScreenSize(0.0001f);
		DecalComponent->SetFadeOut(GetActiveMode().DefaultDecalInfo.DecalFadeOutTime, GetActiveMode().DefaultDecalInfo.DecalLifeTime);
	}
}

void UWeaponBarellComponent::ProcessProjectileHit(AHWProjectile* Projectile, const FHitResult& HitResult,
	const FVector& Direction)
{
	Projectile->SetProjectileActive(false);
	Projectile->SetActorLocation(ProjectilePoolLocation);
	Projectile->SetActorRotation(FRotator::ZeroRotator);
	Projectile->OnProjectileHit.RemoveAll(this);

	ProcessHit(HitResult, Direction);
}

bool UWeaponBarellComponent::HitScan(const FVector& ShotStart, OUT FVector& ShotEnd, const FVector& ShotDirection)
{
	FHitResult HitResult;
	bool bHasHit = GetWorld()->LineTraceSingleByChannel(HitResult, ShotStart, ShotEnd, ECC_Bullet);
	if (bHasHit)
	{
		ShotEnd = HitResult.ImpactPoint;

		ProcessHit(HitResult, ShotDirection);
	}
	return bHasHit;
}

void UWeaponBarellComponent::LaunchProjectile(const FShotInfo& ShotInfo)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	AHWProjectile* Projectile = ProjectilePool[CurrentProjectileIndex];
	bUseCustomFireDamage = ShotInfo.GetCustomDamage(CustomFireDamage);

	check(IsValid(Projectile));
	float CustomProjectileSpeed;
	if (ShotInfo.GetCustomSpeed(CustomProjectileSpeed))
	{
		Projectile->SetInitialSpped(CustomProjectileSpeed);
	}
	Projectile->SetActorLocation(ShotInfo.GetLocation());
	Projectile->SetActorRotation(ShotInfo.GetDirection().ToOrientationRotator());
	Projectile->SetProjectileActive(true);
	Projectile->LaunchProjectile(ShotInfo.GetDirection().GetSafeNormal());
	Projectile->OnProjectileHit.AddDynamic(this, &UWeaponBarellComponent::ProcessProjectileHit);

	CurrentProjectileIndex++;
	if (CurrentProjectileIndex == ProjectilePool.Num())
	{
		CurrentProjectileIndex = 0;
	}
}

APawn* UWeaponBarellComponent::GetOwningPawn() const
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!IsValid(PawnOwner))
	{
		PawnOwner = Cast<APawn>(GetOwner()->GetOwner());
	}
	return PawnOwner;
}

AController* UWeaponBarellComponent::GetController() const
{
	APawn* PawnOwner = GetOwningPawn();
	return IsValid(PawnOwner) ? PawnOwner->GetController() : nullptr;
}

FVector UWeaponBarellComponent::GetBulletSpreadOfset(float Angle, FRotator ShotRotaton) const
{
	float SpreadSize = FMath::Tan(Angle);
	float RotationAngle = FMath::RandRange(0.f, 2 * PI);

	float SpreadY = FMath::Cos(RotationAngle);
	float SpreadZ = FMath::Sin(RotationAngle);

	return (ShotRotaton.RotateVector(FVector::UpVector) * SpreadZ
		+ ShotRotaton.RotateVector(FVector::RightVector) * SpreadY) * SpreadSize;
}
