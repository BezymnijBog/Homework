// Fill out your copyright notice in the Description page of Project Settings.


#include "Bow.h"


#include "Characters/HWBaseCharacter.h"
#include "Components/Weapon/WeaponBarellComponent.h"
#include "Curves/CurveVector.h"

static float InterpParam(float X, float Min, float Max, float DeltaSeconds, float InterpolationTime)
{
	float Alpha = (X - Min) / (Max - Min) + DeltaSeconds / InterpolationTime;
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);
	return FMath::Lerp(Min, Max, Alpha);
}

static void PlayTimelineIfCurveIsValid(UCurveBase* Curve, FTimeline& Timeline, float PlayRate = 1.f)
{
	if (IsValid(Curve))
	{
		Timeline.SetPlayRate(PlayRate);
		Timeline.PlayFromStart();
	}
}

ABow::ABow()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ABow::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateArrowDamage(DeltaSeconds);
	UpdateShakeAngle(DeltaSeconds);
	UpdateShotParams(DeltaSeconds);
	ProcessCameraShake(DeltaSeconds);
}

void ABow::StartAim()
{
	Super::StartAim();
	StartReload();
}

void ABow::StopAim()
{
	Super::StopAim();
	SetActorTickEnabled(false);
	GetWorld()->GetTimerManager().ClearTimer(BowShotTimer);
	ResetParameters();
	
	if (AHWBaseCharacter* Character = GetBaseCharacterOwner())
	{
		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance->Montage_IsPlaying(CharacterReloadMontage))
		{
			float Position = AnimInstance->Montage_GetPosition(CharacterReloadMontage);
			float PlayRate = AnimInstance->Montage_GetPlayRate(CharacterReloadMontage);
			AnimInstance->Montage_Play(CharacterReloadMontage, -PlayRate, EMontagePlayReturnType::Duration, Position);
		}
		else
		{
			AnimInstance->Montage_Play(CharacterReloadMontage, -1.f, EMontagePlayReturnType::Duration, 1.f);
		}
	}

	Unload();
}

void ABow::Unload()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		UnloadInternal();
	}
	Server_Unload();
}

bool ABow::CanShoot() const
{
	return Super::CanShoot() && bStringIsStretched;
}

void ABow::BeginPlay()
{
	Super::BeginPlay();
	OnReloadComplete.AddUObject(this, &ABow::OnStringIsStretched);
	if (IsValid(ShotDataCurve))
	{
		FOnTimelineVectorStatic StringStretchingDelegate;
		StringStretchingDelegate.BindUObject(this, &ABow::UpdateShotParamsWithTimeline);
		StringStretchTimeline.AddInterpVector(ShotDataCurve, StringStretchingDelegate);

		FOnTimelineEventStatic StringIsFullyStretchedDelegate;
		StringIsFullyStretchedDelegate.BindUObject(this, &ABow::OnStringIsFullyStretched);
		StringStretchTimeline.SetTimelineFinishedFunc(StringIsFullyStretchedDelegate);
	}
	
	if (IsValid(ArrowDamageCurve))
	{
		FOnTimelineFloatStatic ArrowDamageChangeDelegate;
		ArrowDamageChangeDelegate.BindUObject(this, &ABow::UpdateArrowDamageWithTimeline);
		ArrowDamageTimeline.AddInterpFloat(ArrowDamageCurve, ArrowDamageChangeDelegate);
	}
	
	if (IsValid(CameraShakeCurve))
	{
		FOnTimelineFloatStatic CameraShakeAmplitudeDelegate;
		CameraShakeAmplitudeDelegate.BindUObject(this, &ABow::UpdateShakeAngleWithTimeline);
		CameraShakeTimeline.AddInterpFloat(CameraShakeCurve, CameraShakeAmplitudeDelegate);
	}
}

void ABow::MakeShot()
{
	WeaponBarell->SetProjectileSpeed(CurrentArrowSpeed);
	WeaponBarell->SetDamage(CurrentArrowDamage);
	
	Super::MakeShot();

	GetWorld()->GetTimerManager().ClearTimer(BowShotTimer);
	ResetParameters();
}

float ABow::GetCurrentBulletSpreadAngle() const
{
	return FMath::DegreesToRadians(CurrentSpreadAngle);
}

float ABow::GetShakeTimeInterval() const
{
	return 1.f / ShakeFrequency;
}

void ABow::InterpolateShotParams(float DeltaSeconds)
{
	CurrentArrowSpeed = InterpParam(CurrentArrowSpeed, MinArrowSpeed, MaxArrowSpeed, DeltaSeconds, InterpolationTime);
	CurrentSpreadAngle = InterpParam(CurrentSpreadAngle, AimSpreadAngle, SpreadAngle, DeltaSeconds, InterpolationTime);
}

void ABow::InterpolateArrowDamage(float DeltaSeconds)
{
	CurrentArrowDamage = InterpParam(CurrentArrowDamage, MinArrowDamage, MaxArrowDamage, DeltaSeconds, MaxArrowHoldTime);
}

void ABow::InterpolateShakeAngle(float DeltaSeconds)
{
	float CameraShakeTime = FMath::Max(MaxArrowHoldTime - NormalHoldArrowTime, 1.f);
	CurrentShakeAngle = InterpParam(CurrentShakeAngle, MinShakeAngle, MaxShakeAngle, DeltaSeconds, CameraShakeTime);
}

void ABow::ProcessCameraShake(float DeltaTime) const
{
	if (!bApplyCameraShake)
	{
		return;
	}

	if (AHWBaseCharacter* Character = GetBaseCharacterOwner())
	{
		if (!Character->IsPlayerControlled() || !Character->IsLocallyControlled())
		{
			return;
		}
		
		float PitchShakeAmplitude = FMath::PerlinNoise1D(GetWorld()->TimeSeconds * 10.f);
		float PitchShake = PitchShakeAmplitude * CurrentShakeAngle * DeltaTime / GetShakeTimeInterval();
		Character->AddControllerPitchInput(PitchShake);
		
		float YawShakeAmplitude = FMath::PerlinNoise1D(GetWorld()->TimeSeconds);
		float YawShake = YawShakeAmplitude * CurrentShakeAngle * DeltaTime / GetShakeTimeInterval();
		Character->AddControllerYawInput(YawShake);
	}
}

void ABow::ResetParameters()
{
	CurrentSpreadAngle = SpreadAngle;
	CurrentArrowSpeed = MinArrowSpeed;
	CurrentArrowDamage = MinArrowDamage;
	CurrentShakeAngle = MinShakeAngle;
	bApplyCameraShake = false;
	bStringIsStretched = false;
	bStringIsFullyStretched = false;
	OnDamageUpdated.ExecuteIfBound(0.f);
}

void ABow::StartCameraShake()
{
	GetWorld()->GetTimerManager().ClearTimer(BowShotTimer);
	if (!IsAiming())
	{
		StopAim();
		return;
	}
	if (GetBaseCharacterOwner() && GetBaseCharacterOwner()->IsLocallyControlled())
	{
		bApplyCameraShake = true;
		float TimeToAutoShot = FMath::Max(MaxArrowHoldTime - NormalHoldArrowTime, 1.f);
		GetWorld()->GetTimerManager().SetTimer(BowShotTimer, this, &ABow::MakeShot, TimeToAutoShot);
		PlayTimelineIfCurveIsValid(CameraShakeCurve, CameraShakeTimeline, 1.f / TimeToAutoShot);
	}
}

void ABow::UnloadInternal()
{
	if (OnUnloaded.IsBound())
	{
		OnUnloaded.Broadcast();
	}
}

void ABow::UpdateArrowDamage(float DeltaSeconds)
{
	if (IsValid(ArrowDamageCurve))
	{
		ArrowDamageTimeline.TickTimeline(DeltaSeconds);
	}
	else
	{
		InterpolateArrowDamage(DeltaSeconds);
	}
	OnDamageUpdated.ExecuteIfBound(CurrentArrowDamage / MaxArrowDamage);
}

void ABow::UpdateShotParams(float DeltaSeconds)
{
	if (IsValid(ShotDataCurve))
	{
		StringStretchTimeline.TickTimeline(DeltaSeconds);
	}
	else
	{
		InterpolateShotParams(DeltaSeconds);
	}
}

void ABow::UpdateShakeAngle(float DeltaSeconds)
{
	if (!bApplyCameraShake)
	{
		return;
	}
	
	if (IsValid(CameraShakeCurve))
	{
		CameraShakeTimeline.TickTimeline(DeltaSeconds);
	}
	else
	{
		InterpolateShakeAngle(DeltaSeconds);
	}
}

void ABow::OnStringIsStretched()
{
	if (!IsAiming())
	{
		StopAim();
		return;
	}
	bStringIsStretched = true;
	SetActorTickEnabled(true);
	if (AHWBaseCharacter* Character = GetBaseCharacterOwner())
	{
		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		float Duration = AnimInstance->Montage_Play(OverdrawMontage);
		InterpolationTime = Duration;
		GetWorld()->GetTimerManager().SetTimer(BowShotTimer, this, &ABow::OnStringIsFullyStretched, Duration);
		PlayTimelineIfCurveIsValid(ShotDataCurve, StringStretchTimeline, 1.f / Duration);
		PlayTimelineIfCurveIsValid(ArrowDamageCurve, ArrowDamageTimeline, 1.f / (Duration + MaxArrowHoldTime));
	}
}

void ABow::OnStringIsFullyStretched()
{
	if (!IsAiming())
	{
		StopAim();
		return;
	}
	bStringIsFullyStretched = true;
	GetWorld()->GetTimerManager().ClearTimer(BowShotTimer);
	GetWorld()->GetTimerManager().SetTimer(BowShotTimer, this, &ABow::StartCameraShake, NormalHoldArrowTime);
}

void ABow::UpdateShakeAngleWithTimeline(float Alpha)
{
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);
	CurrentShakeAngle = FMath::Lerp(MinShakeAngle, MaxShakeAngle, Alpha);
}

void ABow::UpdateArrowDamageWithTimeline(float Alpha)
{
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);
	CurrentArrowDamage = FMath::Lerp(MinArrowDamage, MaxArrowDamage, Alpha);
}

void ABow::UpdateShotParamsWithTimeline(FVector AlphaVector)
{
	AlphaVector.X = FMath::Clamp(AlphaVector.X, 0.f, 1.f);
	AlphaVector.Y = FMath::Clamp(AlphaVector.Y, 0.f, 1.f);
	CurrentSpreadAngle = FMath::Lerp(SpreadAngle, AimSpreadAngle, AlphaVector.X);
	CurrentArrowSpeed = FMath::Lerp(MinArrowSpeed, MaxArrowSpeed, AlphaVector.Y);
}

void ABow::Server_Unload_Implementation()
{
	UnloadInternal();
}
