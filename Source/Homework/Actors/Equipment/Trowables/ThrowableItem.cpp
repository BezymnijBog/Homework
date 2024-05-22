// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowableItem.h"

#include "Actors/Projectiles/HWProjectile.h"
#include "Characters/HWBaseCharacter.h"
#include "Net/UnrealNetwork.h"

void AThrowableItem::Throw()
{
	AHWBaseCharacter* CharacterOwner = GetBaseCharacterOwner();
	if (!IsValid(CharacterOwner))
	{
		return;
	}
	AController* Controller = CharacterOwner->GetController<AController>();
	if (!IsValid(Controller))
	{
		return;
	}

	FVector PlayerViewPoint;
	FRotator PlayerViewRotation;
	Controller->GetPlayerViewPoint(PlayerViewPoint, PlayerViewRotation);
	FTransform PlayerViewTransform(PlayerViewRotation, PlayerViewPoint);

	FVector ViewDirection = PlayerViewRotation.RotateVector(FVector::ForwardVector);
	FVector ViewUpVector = PlayerViewRotation.RotateVector(FVector::UpVector);

	FVector LaunchDirection = ViewDirection + FMath::Tan(FMath::DegreesToRadians(ThrowAngle)) * ViewUpVector;

	FVector ThrowablecSocketLocation = CharacterOwner->GetMesh()->GetSocketLocation(SocketCharacterThrowable);
	FVector SocketInViewSpace = PlayerViewTransform.InverseTransformPosition(ThrowablecSocketLocation);

	FVector SpawnLocation = PlayerViewPoint + ViewDirection * SocketInViewSpace.X;
	
	FShotInfo ThrowInfo(SpawnLocation, LaunchDirection.GetSafeNormal());
	Server_Throw(ThrowInfo);
}

void AThrowableItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	FDoRepLifetimeParams RepParams;
	RepParams.Condition = COND_SimulatedOnly;
	RepParams.RepNotifyCondition = REPNOTIFY_Always;
	DOREPLIFETIME_WITH_PARAMS(AThrowableItem, CurrentGrenadeIdx, RepParams);
	DOREPLIFETIME_WITH_PARAMS(AThrowableItem, GrenadePool, RepParams);
	DOREPLIFETIME_WITH_PARAMS(AThrowableItem, LastThrowInfo, RepParams);
}

void AThrowableItem::BeginPlay()
{
	Super::BeginPlay();
	CreateProjectilePool();
}

void AThrowableItem::CreateProjectilePool()
{
	ClearProjectilePool();

	check(IsValid(ProjectileClass));

	if (GetLocalRole() == ROLE_Authority)
	{
		GrenadePool.Reserve(GrenadePoolSize);
		for (int32 i = 0; i < GrenadePoolSize; ++i)
		{
			AHWProjectile* Grenade = GetWorld()->SpawnActor<AHWProjectile>(ProjectileClass, GrenadePoolLocation, FRotator::ZeroRotator);
			Grenade->SetOwner(GetOwner());
			Grenade->SetProjectileActive(false);
			GrenadePool.Add(Grenade);
		}
	}
}

void AThrowableItem::ClearProjectilePool()
{
	while (GrenadePool.Num() > 0)
	{
		if (IsValid(GrenadePool.Last()))
		{
			GrenadePool.Last()->Destroy();
		}
		GrenadePool.Pop();
	}
}

void AThrowableItem::LaunchGrenade(const FVector& LaunchStart, const FVector& LaunchDirection)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	AHWProjectile* Grenade = GrenadePool[CurrentGrenadeIdx];

	check(IsValid(Grenade));

	Grenade->SetOwner(GetOwner());
	Grenade->SetReturnLocation(GrenadePoolLocation);
	Grenade->SetActorLocation(LaunchStart);
	Grenade->SetActorRotation(LaunchDirection.ToOrientationRotator());
	Grenade->SetProjectileActive(true);
	Grenade->LaunchProjectile(LaunchDirection.GetSafeNormal());

	CurrentGrenadeIdx++;
	if (CurrentGrenadeIdx == GrenadePool.Num())
	{
		CurrentGrenadeIdx = 0;
	}
}

void AThrowableItem::Server_Throw_Implementation(const FShotInfo& ThrowInfo)
{
	ThrowInternal(ThrowInfo);
}

void AThrowableItem::ThrowInternal(const FShotInfo& ThrowInfo)
{
	if (HasAuthority())
	{
		LastThrowInfo = ThrowInfo;
	}

	LaunchGrenade(ThrowInfo.GetLocation(), ThrowInfo.GetDirection());
}

void AThrowableItem::OnRep_LastThrowInfo()
{
	ThrowInternal(LastThrowInfo);
}
