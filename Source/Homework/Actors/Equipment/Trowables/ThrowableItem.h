// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Actors/Equipment/EquipableItem.h"
#include "Actors/Projectiles/HWProjectile.h"

#include "ThrowableItem.generated.h"

class AHWProjectile;

UCLASS(Blueprintable)
class HOMEWORK_API AThrowableItem : public AEquipableItem
{
	GENERATED_BODY()

public:
	void Throw();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwables")
	TSubclassOf<AHWProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thrawables", meta = (UIMin = -90.f, UIMax = 90.f, ClampMin = -90.f, ClampMax = 90.f))
	float ThrowAngle = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Thrawables", meta = (UIMin = 1, ClampMin = 1))
	int32 GrenadePoolSize = 5;

private:
	void CreateProjectilePool();
	void ClearProjectilePool();
	void LaunchGrenade(const FVector& LaunchStart, const FVector& LaunchDirection);

	UFUNCTION(Server, Reliable)
	void Server_Throw(const FShotInfo& ThrowInfo);
	void Server_Throw_Implementation(const FShotInfo& ThrowInfo);

	void ThrowInternal(const FShotInfo& ThrowInfo);

	UFUNCTION()
	void OnRep_LastThrowInfo();
	
	UPROPERTY(ReplicatedUsing=OnRep_LastThrowInfo)
	FShotInfo LastThrowInfo;
	
	UPROPERTY(Replicated)
	TArray<AHWProjectile*> GrenadePool;
	
	UPROPERTY(Replicated)
	int32 CurrentGrenadeIdx;

	const FVector GrenadePoolLocation = {0.f, 0.f, -100.f};
};
