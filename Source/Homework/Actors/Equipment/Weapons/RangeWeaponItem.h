// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Equipment/EquipableItem.h"
#include "RangeWeaponItem.generated.h"

class UWeaponBarellComponent;
class AHWBaseCharacter;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAmmoChanged, int32)
DECLARE_MULTICAST_DELEGATE(FOnReloadComplete)
DECLARE_MULTICAST_DELEGATE(FOnUnloaded)

UENUM()
enum class EReloadType : uint8
{
	FullClip,
	ByBullet
};

UCLASS(Blueprintable)
class HOMEWORK_API ARangeWeaponItem : public AEquipableItem
{
	GENERATED_BODY()

public:
	ARangeWeaponItem();
	
	// void modifying methods:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void StartFire();
	void StopFire();
	virtual void StartAim();
	virtual void StopAim();
	void StartReload();
	void StartReloadInternal(bool bClientSimulation = false);
	void EndReload(bool bIsSuccess);
	void EndReloadInternal(bool bIsSuccess, bool bClientSimulation = false);

	void SwitchBarellMode();
	void RemoveAmmo(EAmunitionType AmunitionType);
	
	// Getters:
	
	float GetAimFOV() const;
	float GetAimMaxMoveSpeed() const;
	float GetTurnModifier() const;
	float GetLookUpModifier() const;
	int32 GetAmmo() const;
	int32 GetMaxAmmo() const;
	EAmunitionType GetAmmoType() const;
	virtual EReticleType GetReticleType() const override;
	FTransform GetForeGripTransform() const;
	
	// Setters:
	
	void SetAmmo(int32 NewAmmo);
	
	// bool checking methods:
	
	virtual bool CanShoot() const;
	bool IsAiming() const;
	bool IsFiring()	const;
	bool IsReloading() const;

	// Delegates:

	FOnAmmoChanged OnAmmoChanged;
	FOnReloadComplete OnReloadComplete;
	FOnUnloaded OnUnloaded;
	
protected:
	// protected methods:
	virtual void MakeShot();

	// UFUNCTIONS:

	// UPROPERTIES:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWeaponBarellComponent* WeaponBarell;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations | Weapon")
	UAnimMontage* WeaponFireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations | Weapon")
	UAnimMontage* WeaponReloadMontage;

	// FullClip reload type adds ammo when the whole reload animation is successfully played
	// ByBullet reload type requires section "ReloadEnd" in character reload animation
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations | Weapon")
	EReloadType ReloadType = EReloadType::FullClip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations | Character")
	UAnimMontage* CharacterFireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations | Character")
	UAnimMontage* CharacterReloadMontage;

	// Bullet spread half-angle in degrees
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters", meta = (ClampMin = 0.f, UIMin = 0.f, ClampMax = 5.f, UIMax = 5.f))
	float SpreadAngle = 1.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming", meta = (ClampMin = 0.f, UIMin = 0.f, ClampMax = 5.f, UIMax = 5.f))
	float AimSpreadAngle = 0.25f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float AimMaxSpeed = 200.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float AimFOV = 60.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming", meta = (ClampMin = 0.f, UIMin = 0.f, ClampMax = 1.f, UIMax = 1.f))
	float AimTurnModifier = 0.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming", meta = (ClampMin = 0.f, UIMin = 0.f, ClampMax = 1.f, UIMax = 1.f))
	float AimLookUpModifier = 0.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Ammo")
	bool bAutoReload = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reticle")
	EReticleType AimReticleType = EReticleType::Default;

private:
	virtual float GetCurrentBulletSpreadAngle() const;
	float GetShotTimerInterval() const;
	void TryAutoReload(AHWBaseCharacter* CharacterOwner);
	void OnShotTimerElapsed();
	float PlayAnimMontage(UAnimMontage* AnimMontage) const;
	void StopAnimMontage(UAnimMontage* AnimMontage, float BlendOutTime = 0.f);

	UFUNCTION(Server, Reliable)
	void Server_StartReload();
	
	UFUNCTION(Server, Reliable)
	void Server_EndReload(bool bIsSuccess);
	
	UPROPERTY(ReplicatedUsing=OnRep_Reloading)
	bool bIsReloading = false;
	UPROPERTY(Replicated)
	bool bWasLstReloadSuccessful = false;
	UFUNCTION()
	void OnRep_Reloading(bool bIsReloading_Old);
	
	FTimerHandle ReloadTimer;
	FTimerHandle ShotTimer;
	uint32 ActiveBarell = 0;

	bool bIsAiming = false;
	bool bIsFiring = false;
};
