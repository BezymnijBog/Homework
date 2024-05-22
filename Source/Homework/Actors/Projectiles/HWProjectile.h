// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HWProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class AHWProjectile;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnProjectileHit, AHWProjectile*, Projectile, const FHitResult&, HitResult, const FVector&, Direction);

USTRUCT(BlueprintType)
struct FShotInfo
{
	GENERATED_BODY()

	FShotInfo()
		: Location_Mul_10(FVector_NetQuantize100::ZeroVector)
		, Direction(FVector_NetQuantizeNormal::ZeroVector)
		, bUseCustomProjectileSpeed(0)
		, bUseCustomShotDamage(0)
		, CustomProjectileSpeed(0.f)
		, CustomShotDamage(0.f)
	{
	}

	FShotInfo(const FVector& Location, const FVector& Direction)
		: Location_Mul_10(Location * 10)
		, Direction(Direction)
		, bUseCustomProjectileSpeed(0)
		, bUseCustomShotDamage(0)
		, CustomProjectileSpeed(0.f)
		, CustomShotDamage(0.f)
	{
	}

	UPROPERTY()
	FVector_NetQuantize100 Location_Mul_10;

	UPROPERTY()
	FVector_NetQuantizeNormal Direction;

	UPROPERTY()
	uint8 bUseCustomProjectileSpeed:1;

	UPROPERTY()
	uint8 bUseCustomShotDamage:1;

	UPROPERTY()
	float CustomProjectileSpeed;
	
	UPROPERTY()
	float CustomShotDamage;

	FVector GetLocation() const { return Location_Mul_10 * 0.1f; }
	FVector GetDirection() const { return Direction; }
	bool GetCustomSpeed(float& Speed) const
	{
		Speed = CustomProjectileSpeed;
		return bUseCustomProjectileSpeed;
	}
	void SetCustomProjectileSpeed(float Speed)
	{
		CustomProjectileSpeed = Speed;
		bUseCustomProjectileSpeed = 1;
	}
	bool GetCustomDamage(float& Damage) const
	{
		Damage = CustomShotDamage;
		return bUseCustomShotDamage;
	}
	void SetCustomShotDamage(float Damage)
	{
		CustomShotDamage = Damage;
		bUseCustomShotDamage = 1;
	}
};

UCLASS()
class HOMEWORK_API AHWProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AHWProjectile();

	void SetInitialSpped(float Speed);
	
	void SetReturnLocation(const FVector& Location);

	UFUNCTION(BlueprintCallable)
	void LaunchProjectile(const FVector& Direction);

	UFUNCTION(BlueprintNativeEvent)
	void SetProjectileActive(bool bIsActive);
	
	UPROPERTY(BlueprintAssignable)
	FOnProjectileHit OnProjectileHit;

protected:
	virtual void BeginPlay() override;
	virtual void OnProjectileLaunched();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(BlueprintReadOnly)
	FVector ProjectileReturnLocation = FVector::ZeroVector;

private:
	UFUNCTION()
	void OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
