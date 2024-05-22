// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "HomeworkTypes.h"
#include "NiagaraComponent.h"

#include "Actors/Projectiles/HWProjectile.h"
#include "Components/SceneComponent.h"
#include "WeaponBarellComponent.generated.h"

struct FShotInfo;
class AHWProjectile;
class UNiagaraSystem;

UENUM(BlueprintType)
enum class EHitRegistrationType : uint8
{
	HitScan,
	Projectile
};

UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
	Single,
	FullAuto
};

USTRUCT(BlueprintType)
struct FDecalInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal info")
	UMaterialInterface* DecalMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal info")
	FVector DecalSize = {5.f, 5.f, 5.f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal info")
	float DecalLifeTime = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal info")
	float DecalFadeOutTime = 5.f;
};

USTRUCT(BlueprintType)
struct FWeaponBarellDescription
{
	GENERATED_BODY()
	
	// Rate of fire in rounds per minute
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 1.f, UIMin = 1.f))
	float RateOfFire = 600.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EWeaponFireMode FireMode = EWeaponFireMode::Single;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EAmunitionType AmmoType = EAmunitionType::None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float FireRange = 5000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 1, ClampMin = 1))
	int32 BulletsPerShot = 1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = 1, UIMin = 1))
	int32 MaxAmmo = 30;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float BaseFireDamage = 20.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UDamageType> DamageTypeClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	UCurveFloat* FalloffDiagram;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* MuzzleFlashFX;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* TraceFX;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decals")
	FDecalInfo DefaultDecalInfo;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit registration")
	EHitRegistrationType HitRegistration = EHitRegistrationType::HitScan;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit registration", meta = (UIMin = 1, ClampMin = 1, EditCondition = "HitRegistration == EHitRegistrationType::Projectile"))
	int32 ProjectilePoolSize = 10;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit registration", meta = (EditCondition = "HitRegistration == EHitRegistrationType::Projectile"))
	TSubclassOf<AHWProjectile> ProjectileClass;

	int32 Ammo = 0;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOMEWORK_API UWeaponBarellComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UWeaponBarellComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void Shot(const FVector& ShotStart, FVector ShotDirection, float SpreadAngle);
	
	void SetAmmo(int32 NewAmmo);
	
	void SetDamage(float Damage);
	
	void SetProjectileSpeed(float Speed);

	bool SwitchBarellMode();
	
	int32 GetAmmo() const;
	
	int32 GetMaxAmmo() const;

	float GetROF() const;

	float GetDamage() const;
	
	EAmunitionType GetAmmoType() const;

	EWeaponFireMode GetFireMode() const;
	void ReserveProjectilePool();
	void RemoveAmmo(EAmunitionType AmunitionType);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barell attributes")
	TArray<FWeaponBarellDescription> BarellModes;

private:
	FWeaponBarellDescription& GetActiveMode();
	const FWeaponBarellDescription& GetActiveMode() const;
	void ShotInternal(const TArray<FShotInfo>& ShotsInfo);
	
	UFUNCTION(Server, Reliable)
	void Server_Shot(const TArray<FShotInfo>& ShotsInfo);

	UPROPERTY(ReplicatedUsing=OnRep_LastShotsInfo)
	TArray<FShotInfo> LastShotsInfo;

	UPROPERTY(Replicated)
	TArray<AHWProjectile*> ProjectilePool;

	UPROPERTY(Replicated)
	int32 CurrentProjectileIndex = 0;

	UPROPERTY(Replicated)
	int32 Ammo;
	int32 ActiveMode = 0;
	
	UPROPERTY(Replicated)
	bool bUseCustomFireDamage = false;
	
	UPROPERTY(Replicated)
	float ProjectileInitialSpeed = -1.f;

	UPROPERTY(Replicated)
	float CustomFireDamage = 0.f;

	UFUNCTION()
	void OnRep_LastShotsInfo();
	
	UFUNCTION()
	void ProcessHit(const FHitResult& HitResult, const FVector& Direction);

	UFUNCTION()
	void ProcessProjectileHit(AHWProjectile* Projectile, const FHitResult& HitResult, const FVector& Direction);
	
	bool HitScan(const FVector& ShotStart, FVector& ShotEnd, const FVector& ShotDirection);
	void LaunchProjectile(const FShotInfo& ShotInfo);

	APawn* GetOwningPawn() const;
	AController* GetController() const;
	FVector GetBulletSpreadOfset(float Angle, FRotator ShotRotaton) const;
	
	const FVector ProjectilePoolLocation = FVector(0.f, 0.f, -100.f);
};
