// Fill out your copyright notice in the Description page of Project Settings.


#include "Turret.h"

#include "AIController.h"
#include "Components/PawnComponents/PawnAttributesComponent.h"
#include "Components/Weapon/WeaponBarellComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AISense_Damage.h"

ATurret::ATurret()
{
	SetReplicates(true);
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* TurretRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Turret Root"));
	SetRootComponent(TurretRoot);

	TurretBaseComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Turret Base"));
	TurretBaseComponent->SetupAttachment(TurretRoot);

	TurretBarellComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Turret Barell"));
	TurretBarellComponent->SetupAttachment(TurretBaseComponent);

	WeaponBarell = CreateDefaultSubobject<UWeaponBarellComponent>(TEXT("Weapon Barell"));
	WeaponBarell->SetupAttachment(TurretBarellComponent);

	TurretAttributes = CreateDefaultSubobject<UPawnAttributesComponent>(TEXT("Turret Attrinutes"));
}

void ATurret::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATurret, CurrentTarget);
}

void ATurret::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AAIController* AIController = Cast<AAIController>(NewController);
	if (IsValid(AIController))
	{
		uint8 TeamValue = StaticCast<uint8>(Team);
		FGenericTeamId TeamId(TeamValue);
		AIController->SetGenericTeamId(TeamId);
	}
}

float ATurret::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ResultDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (IsValid(DamageCauser)) 
	{
		UAISense_Damage::ReportDamageEvent(GetWorld(), this, DamageCauser, DamageAmount, DamageCauser->GetActorLocation(), GetActorLocation());
	}
	return ResultDamage;
}

void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (CurrentState)
	{
	case ETurretState::Searching:
		SearchingMovement(DeltaTime);
		break;
	case ETurretState::Firing:
		FiringMovement(DeltaTime);
		break;
	case ETurretState::RotateToStimulus:
		StimulusRotation(DeltaTime);
		break;
	default:
		break;
	}
}

void ATurret::OnCurrentTargetSet()
{
	ETurretState NewState = CurrentTarget.IsValid() ? ETurretState::Firing : ETurretState::Searching;
	SetCurrentState(NewState);
}

FVector ATurret::GetPawnViewLocation() const
{
	return WeaponBarell->GetComponentLocation();
}

FRotator ATurret::GetViewRotation() const
{
	return WeaponBarell->GetComponentRotation();
}

void ATurret::RotateToStimulus(FVector StimulusLocation)
{
	FVector TurretLocation = GetActorLocation();
	FVector DirectionToStimulus = (StimulusLocation - TurretLocation).GetSafeNormal();
	DirectionToLastStimulus = DirectionToStimulus;
	SetCurrentState(ETurretState::RotateToStimulus);
}

void ATurret::BeginPlay()
{
	Super::BeginPlay();
	TurretAttributes->OnDeathDelegate.AddUObject(this, &ATurret::OnDeath);
}

void ATurret::OnDeath_Implementation()
{
	if (IsValid(OnDeathVFX))
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OnDeathVFX, GetActorLocation());
	}
	CurrentTarget = nullptr;
	DirectionToLastStimulus = FVector::ZeroVector;
	GetController()->UnPossess();
	SetCurrentState(ETurretState::Dead);
}

void ATurret::OnRep_CurrentTarget()
{
	OnCurrentTargetSet();
}

void ATurret::MakeShot()
{
	FVector ShotStart = WeaponBarell->GetComponentLocation();
	FVector ShotDirection = WeaponBarell->GetComponentRotation().RotateVector(FVector::ForwardVector);
	float SpreadAngle = FMath::DegreesToRadians(BulletSpreadAngle);
	WeaponBarell->Shot(ShotStart, ShotDirection, SpreadAngle);
}

void ATurret::FiringMovement(float DeltaTime)
{
	FVector BaseLookAtDirection = (CurrentTarget->GetActorLocation() - TurretBaseComponent->GetComponentLocation()).GetSafeNormal2D();
	FQuat LookAtQuat = BaseLookAtDirection.ToOrientationQuat();
	FQuat TargetQuat = FMath::QInterpTo(TurretBaseComponent->GetComponentQuat(), LookAtQuat, DeltaTime, BaseFiringInterpSpeed);
	TurretBaseComponent->SetWorldRotation(TargetQuat);

	FVector BarellLookAtDirection = (CurrentTarget->GetActorLocation() - TurretBarellComponent->GetComponentLocation()).GetSafeNormal();
	float LookAtPitchAngle = BarellLookAtDirection.ToOrientationRotator().Pitch;

	FRotator BarellRelativeRotation = TurretBarellComponent->GetRelativeRotation();
	BarellRelativeRotation.Pitch = FMath::FInterpTo(BarellRelativeRotation.Pitch, LookAtPitchAngle, DeltaTime, BarellPitchRotationRate);
	TurretBarellComponent->SetRelativeRotation(BarellRelativeRotation);
}

void ATurret::SearchingMovement(float DeltaTime)
{
	FRotator TurretBaseRotation = TurretBaseComponent->GetRelativeRotation();
	TurretBaseRotation.Yaw += DeltaTime * BaseSearchingRotationRate;
	TurretBaseComponent->SetRelativeRotation(TurretBaseRotation);

	FRotator TurretBarellRotation = TurretBarellComponent->GetRelativeRotation();
	TurretBarellRotation.Pitch = FMath::FInterpTo(TurretBarellRotation.Pitch, 0.f, DeltaTime, BarellPitchRotationRate);
	TurretBarellComponent->SetRelativeRotation(TurretBarellRotation);
}

void ATurret::StimulusRotation(float DeltaTime)
{
	FQuat LookAtQuat = DirectionToLastStimulus.ToOrientationQuat();
	FQuat TargetQuat = FMath::QInterpTo(TurretBaseComponent->GetComponentQuat(), LookAtQuat, DeltaTime, StimulusSearchingInterpSpeed);
	TurretBaseComponent->SetWorldRotation(TargetQuat);
	
	if (!StimulusSearchTimer.IsValid() && LookAtQuat.Equals(TargetQuat))
	{
		GetWorld()->GetTimerManager().SetTimer(StimulusSearchTimer, [this]() { SetCurrentState(ETurretState::Searching); }, StimulusSearchingTime, false);
	}
}

void ATurret::SetCurrentState(ETurretState NewState)
{
	bool bIsStateChanged = NewState != CurrentState;
	CurrentState = NewState;

	if (!bIsStateChanged)
	{
		return;
	}
	
	GetWorld()->GetTimerManager().ClearTimer(StimulusSearchTimer);
	switch (CurrentState)
	{
	case ETurretState::Searching:
	case ETurretState::RotateToStimulus:
		GetWorld()->GetTimerManager().ClearTimer(ShotTimer);
		break;
	case ETurretState::Firing:
		GetWorld()->GetTimerManager().SetTimer(ShotTimer, this, &ATurret::MakeShot, GetFireInterval(), true, FireDelayTime);
		break;
	case ETurretState::Dead:
		GetWorld()->GetTimerManager().ClearTimer(ShotTimer);
		SetActorTickEnabled(false);
		break;
	}
}

float ATurret::GetFireInterval() const
{
	return 60.f / RateOfFire;
}

