// Fill out your copyright notice in the Description page of Project Settings.


#include "HWBaseCharacter.h"

#include "AIController.h"
#include "Actors/Environment/PlatformTrigger.h"
#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Actors/Interactive/Environment/Ladder.h"
#include "Actors/Interactive/Environment/Zipline.h"
#include "Actors/Interactive/InteractiveActor.h"
#include "Actors/Interactive/Interface/Interactive.h"
#include "Components/CapsuleComponent.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"
#include "Components/LedgeDetectorComponent.h"
#include "Components/MovementComponents/HWBaseCharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Curves/CurveVector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "HomeworkTypes.h"
#include "Inventory/Items/InventoryItem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "UI/Widget/World/HWAttributeProgressBar.h"


AHWBaseCharacter::AHWBaseCharacter(const FObjectInitializer& InObjectInitializer)
	:Super(InObjectInitializer.SetDefaultSubobjectClass<UHWBaseCharacterMovementComponent>(CharacterMovementComponentName))
{
	BaseCharacterMovementComponent = StaticCast<UHWBaseCharacterMovementComponent*>(GetCharacterMovement());

	CharacterAttributesComponent = CreateDefaultSubobject<UCharacterAttributesComponent>("Character attributes");
	LedgeDetectorComponent = CreateDefaultSubobject<ULedgeDetectorComponent>("Ledge detector");
	CharacterInventoryComponent = CreateDefaultSubobject<UCharacterInventoryComponent>("Inventory component");
	CharacterEquipmentComponent = CreateDefaultSubobject<UCharacterEquipmentComponent>("Character equipment");

	GetMesh()->CastShadow = 1;
	GetMesh()->bCastDynamicShadow = 1;

	HealthBarProgressComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarProgressComponent"));
	HealthBarProgressComponent->SetupAttachment(GetCapsuleComponent());
}

void AHWBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHWBaseCharacter, bIsMantling)
	DOREPLIFETIME(AHWBaseCharacter, bIsSliding)
	DOREPLIFETIME(AHWBaseCharacter, bIsAiming)
}

void AHWBaseCharacter::PossessedBy(AController* NewController)
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

void AHWBaseCharacter::MoveForward(float InValue)
{
	ForwardAxis = InValue;
}

void AHWBaseCharacter::MoveRight(float InValue)
{
	RightAxis = InValue;
}

void AHWBaseCharacter::ClimbLadderUp(float InValue)
{
	if (GetBaseCharacterMovement()->IsOnLadder() && !FMath::IsNearlyZero(InValue))
	{
		FVector LadderDirection = GetBaseCharacterMovement()->GetCurrentLadder()->GetActorUpVector();
		AddMovementInput(LadderDirection, InValue);
	}
}

void AHWBaseCharacter::InteractWithLadder()
{
	if (!CanInteractWithLadder())
	{
		return;
	}
	
	if (GetBaseCharacterMovement()->IsOnLadder())
	{
		GetBaseCharacterMovement()->DetachFromLadder(EDetachFromLadderMethod::JumpOff);
	}
	else
	{
		const ALadder* AvailableLadder = GetAvailable<ALadder>();
		if (IsValid(AvailableLadder))
		{
			if (AvailableLadder->IsOnTop())
			{
				PlayAnimMontage(AvailableLadder->GetAttachFromTopAnimMontage());
			}
			GetBaseCharacterMovement()->AttachToLadder(AvailableLadder);
		}
	}
}

void AHWBaseCharacter::ZiplineInteraction()
{
	if (GetBaseCharacterMovement()->IsOnZipline())
	{
		GetBaseCharacterMovement()->DetachFromZipline(true);
	}
	else
	{
		const AZipline* AvailableZipline = GetAvailable<AZipline>();
		if (IsValid(AvailableZipline))
		{
			GetBaseCharacterMovement()->AttachToZipline(AvailableZipline);
		}
	}
}

void AHWBaseCharacter::Jump()
{
	if (BaseCharacterMovementComponent->IsProne())
	{
		StandUp();
		UnCrouch();
	}
	else if (GetBaseCharacterMovement()->IsWallRunning())
	{
		EWallRunSide CurrentSide = GetBaseCharacterMovement()->GetWallRunSide();
		FVector MoveDirection = GetCharacterMovement()->Velocity;
		MoveDirection.Normalize();
		FVector JumpDirection = FVector::UpVector;
		if (CurrentSide == EWallRunSide::Right) 
		{
			JumpDirection += FVector::CrossProduct(MoveDirection, FVector::UpVector).GetSafeNormal();
		}
		else 
		{
			JumpDirection += FVector::CrossProduct(FVector::UpVector, MoveDirection).GetSafeNormal();
		}
		float LaunchSpeed = GetCharacterMovement()->JumpZVelocity;
		LaunchCharacter(LaunchSpeed * JumpDirection.GetSafeNormal(), false, true);
		GetBaseCharacterMovement()->EndWallRun();
	}
	else
	{
		Super::Jump();
	}
}

void AHWBaseCharacter::Mantle(bool bForse/* = false*/)
{
	if (!CanMantle() && !bForse)
	{
		return;
	}
	FLedgeDescription LedgeDescription;
	if (LedgeDetectorComponent->DetectLedge(LedgeDescription))
	{
		bIsMantling = true;
		
		// set parameters, that we have
		FMantlingMovementParameters MantlingParameters;
		MantlingParameters.MantlingCurve = HighMantleSettings.MantlingCurve;
		MantlingParameters.InitialLocation = GetActorLocation() - LedgeDescription.LedgeComponent->GetComponentLocation();
		MantlingParameters.InitialRotation = GetActorRotation();
		MantlingParameters.TargetLocation = LedgeDescription.Location - LedgeDescription.LedgeComponent->GetComponentLocation();
		MantlingParameters.TargetRotation = LedgeDescription.Rotation;
		MantlingParameters.TargetComponent = LedgeDescription.LedgeComponent;
		// if is crouching - uncrouch and correct initial location
		if (bIsCrouched)
		{
			UnCrouch();
			const float DeltaHeight = GetDefaultHalfHeight() - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			MantlingParameters.InitialLocation.Z += DeltaHeight;
		}
		const float CharacterBottomZ = GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const float MantlingHeight = LedgeDescription.LedgeBottom.Z - CharacterBottomZ;
		const FMantlingSettings& MantlingSettings = GetMantlingSettings(MantlingHeight);
		// calculate mantling duration
		float MinRange;
		float MaxRange;
		MantlingSettings.MantlingCurve->GetTimeRange(MinRange, MaxRange);
		MantlingParameters.Duration = MaxRange - MinRange;
		// calculate start time for montage
		const FVector2D SourceRange(MantlingSettings.MinHeight, MantlingSettings.MaxHeight);
		const FVector2D TargetRange(MantlingSettings.MinHeightStartTime, MantlingSettings.MaxHeightStartTime);
		MantlingParameters.StartTime = FMath::GetMappedRangeValueClamped(SourceRange, TargetRange, MantlingHeight);
		// start location as if we mantle on highest possible height
		MantlingParameters.InitialAnimationLocation = MantlingParameters.TargetLocation - MantlingSettings.AnimationCorrectionZ * FVector::UpVector + MantlingSettings.AnimationCorrectionXY * LedgeDescription.LedgeNormal;
		// start mantling and play montage
		if (IsLocallyControlled() || GetLocalRole() == ROLE_Authority)
		{
			GetBaseCharacterMovement()->StartMantle(MantlingParameters);
		}
		OnMantle(MantlingSettings, MantlingParameters.StartTime);
	}
}

void AHWBaseCharacter::StartSlide()
{
	bIsSlideRequested = true;
}

void AHWBaseCharacter::StopSlide()
{
	bIsSlideRequested = false;
}

void AHWBaseCharacter::ChangeCrouchState()
{
	if (GetCharacterMovement()->IsCrouching())
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AHWBaseCharacter::ChangeProneState()
{
	if (BaseCharacterMovementComponent->IsProne())
	{
		StandUp();
	}
	else
	{
		LieDown();
	}
}

void AHWBaseCharacter::StartSprint()
{
	bIsSprintRequested = true;
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void AHWBaseCharacter::StopSprint()
{
	bIsSprintRequested = false;
}

bool AHWBaseCharacter::IsSprinting() const
{
	return GetBaseCharacterMovement()->IsSprinting() && !bIsSliding;
}

void AHWBaseCharacter::StartFire()
{
	if (!CanFire())
	{
		return;
	}
	ARangeWeaponItem* CurrentRangeWeapon = GetEquipmentComponent()->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StartFire();
	}
}

void AHWBaseCharacter::StopFire()
{
	ARangeWeaponItem* CurrentRangeWeapon = GetEquipmentComponent()->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StopFire();
	}
}

void AHWBaseCharacter::StartAim()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_StartAim();
	}
	StartAimInternal();
}

void AHWBaseCharacter::StopAim()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_StopAim();
	}
	StopAimInternal();
}

float AHWBaseCharacter::GetAimingMoveSpeed() const
{
	return CurrentAimingMovementSpeed;
}

bool AHWBaseCharacter::IsAiming() const
{
	return bIsAiming;
}

void AHWBaseCharacter::Reload()
{
	if (IsValid(GetEquipmentComponent()->GetCurrentRangeWeapon()))
	{
		CharacterEquipmentComponent->ReloadCurrentWeapon();
	}
}

void AHWBaseCharacter::NexItem()
{
	CharacterEquipmentComponent->EquipNextItem();
}

void AHWBaseCharacter::PreviousItem()
{
	CharacterEquipmentComponent->EquipPreviousItem();
}

void AHWBaseCharacter::EquipPrimaryItem()
{
	CharacterEquipmentComponent->EquipItemInSlot(EEquipmentSlots::PrimaryItemSlot);
}

void AHWBaseCharacter::PrimaryMeleeAttack()
{
	AMeleeWeaponItem* CurrentMeleWeapon = GetEquipmentComponent()->GetCurrentMeleeWeapon();
	if (IsValid(CurrentMeleWeapon))
	{
		CurrentMeleWeapon->StartAttack(EMeleeAttackTypes::PrimaryAttack);
	}
}

void AHWBaseCharacter::SecondaryMeleeAttack()
{
	AMeleeWeaponItem* CurrentMeleWeapon = GetEquipmentComponent()->GetCurrentMeleeWeapon();
	if (IsValid(CurrentMeleWeapon))
	{
		CurrentMeleWeapon->StartAttack(EMeleeAttackTypes::SecondaryAttack);
	}
}

void AHWBaseCharacter::Interact()
{
	if (LineOfSightObject.GetInterface())
	{
		LineOfSightObject->Interact(this);
	}
}

void AHWBaseCharacter::UseInventory(APlayerController* PlayerController)
{
	if (!IsValid(PlayerController))
	{
		return;
	}

	if (!CharacterInventoryComponent->IsViewVisible())
	{
		CharacterInventoryComponent->OpenViewInventory(PlayerController);
		CharacterEquipmentComponent->OpenViewEquipment(PlayerController);
		PlayerController->SetInputMode(FInputModeGameAndUI());
		PlayerController->bShowMouseCursor = true;
	}
	else
	{
		CharacterInventoryComponent->CloseViewInventory();
		CharacterEquipmentComponent->CloseViewEquipment();
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->bShowMouseCursor = false;
	}
}

void AHWBaseCharacter::SwitchWeaponMode()
{
	ARangeWeaponItem* RangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (IsValid(RangeWeapon))
	{
		RangeWeapon->SwitchBarellMode();
	}
}

void AHWBaseCharacter::DestroyActorOnServer(AActor* Actor) const
{
	Server_DestroyActor(Actor);
}

bool AHWBaseCharacter::CanFire() const
{
	return !GetEquipmentComponent()->IsEquipping() && !CharacterEquipmentComponent->IsSelectingWeapon();
}

bool AHWBaseCharacter::PickupItem(UInventoryItem* Item)
{
	bool bResult = false;
	if (CharacterInventoryComponent->HasSlotForItem(Item))
	{
		bResult |= CharacterInventoryComponent->AddItem(Item, Item->GetCount());
	}
	return bResult;
}

void AHWBaseCharacter::ConfirmWeaponSelection()
{
	if (CharacterEquipmentComponent->IsSelectingWeapon())
	{
		CharacterEquipmentComponent->ConfirmWeaponSelection();
	}
}

void AHWBaseCharacter::OnLevelDeserialized_Implementation()
{
}

void AHWBaseCharacter::OnStartAim_Implementation()
{
	OnStartAimInternal();
}

void AHWBaseCharacter::OnStoptAim_Implementation()
{
	OnStopAimInternal();
}

void AHWBaseCharacter::Falling()
{
	// base method is empty, so no need to call it
	GetBaseCharacterMovement()->bNotifyApex = true;
}

void AHWBaseCharacter::NotifyJumpApex()
{
	Super::NotifyJumpApex();
	CurrentFallApex = GetActorLocation();
}

void AHWBaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	float FallHeight = (CurrentFallApex - GetActorLocation()).Z;
	if (IsValid(FallDamageCurve))
	{
		float Damage = FallDamageCurve->GetFloatValue(FallHeight);
		TakeDamage(Damage, FDamageEvent(), Controller, Hit.GetActor());
	}
	if (FallHeight > HardLandingHeight && IsValid(HardLandingMontage))
	{
		PlayAnimMontage(HardLandingMontage);
	}
}

void AHWBaseCharacter::RegisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvailableInteractiveActors.AddUnique(InteractiveActor);
}

void AHWBaseCharacter::UnRegisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvailableInteractiveActors.RemoveSingleSwap(InteractiveActor);
}

bool AHWBaseCharacter::IsSwimmingUnderWater() const
{
	if (!GetCharacterMovement()->IsSwimming())
	{
		return false;
	}
	FVector HeadPosition = GetMesh()->GetSocketLocation(BoneHead);
	APhysicsVolume* Volume = GetCharacterMovement()->GetPhysicsVolume();
	float VolumeTopPlane = Volume->GetActorLocation().Z + Volume->GetBounds().BoxExtent.Z * Volume->GetActorScale3D().Z;
	return VolumeTopPlane > HeadPosition.Z;
}

bool AHWBaseCharacter::AreRequiredKeysDown(EWallRunSide Side) const
{
	bool bIsTooSlow = ForwardAxis < 0.1f;
	bool bIsMovedAway = Side == EWallRunSide::Right && RightAxis < -0.1f || Side == EWallRunSide::Left && RightAxis > 0.1f;
	return !bIsTooSlow && !bIsMovedAway;
}

void AHWBaseCharacter::GetWallRunSideAndDirection(const FVector& HitNormal, EWallRunSide& OutSide, FVector& OutDirection) const
{
	if (FVector::DotProduct(HitNormal, GetActorRightVector()) > 0)
	{
		OutSide = EWallRunSide::Left;
		OutDirection = FVector::CrossProduct(HitNormal, FVector::UpVector).GetSafeNormal();
	}
	else
	{
		OutSide = EWallRunSide::Right;
		OutDirection = FVector::CrossProduct(FVector::UpVector, HitNormal).GetSafeNormal();
	}
}

void AHWBaseCharacter::OnStartSliding(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	RecalculateBaseEyeHeight();

	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent && DefaultChar->GetMesh())
	{
		FVector& MeshRelativeLocation = MeshComponent->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z = DefaultChar->GetMesh()->GetRelativeLocation().Z + HalfHeightAdjust;
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->GetBaseTranslationOffset().Z + HalfHeightAdjust;
	}

	if (UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance())
	{
		if (!AnimInstance->IsAnyMontagePlaying())
		{
			AnimInstance->Montage_Play(SlidingAnimMontage);
		}
	}

	K2_OnStartSliding(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void AHWBaseCharacter::OnEndSliding(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	RecalculateBaseEyeHeight();

	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent && DefaultChar->GetMesh())
	{
		FVector& MeshRelativeLocation = MeshComponent->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z = DefaultChar->GetMesh()->GetRelativeLocation().Z;
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->GetBaseTranslationOffset().Z;
	}
	K2_OnEndSliding(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void AHWBaseCharacter::OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	RecalculateBaseEyeHeight();

	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent && DefaultChar->GetMesh())
	{
		FVector& MeshRelativeLocation = MeshComponent->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z = DefaultChar->GetMesh()->GetRelativeLocation().Z;
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->GetBaseTranslationOffset().Z;
	}
	
	K2_OnEndProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void AHWBaseCharacter::OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	RecalculateBaseEyeHeight();

	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent && DefaultChar->GetMesh())
	{
		FVector& MeshRelativeLocation = MeshComponent->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z = DefaultChar->GetMesh()->GetRelativeLocation().Z + HalfHeightAdjust;
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->GetBaseTranslationOffset().Z + HalfHeightAdjust;
	}
	
	K2_OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void AHWBaseCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	TryChangeSprintState();
	TryChangeSlideState();
	
	IKLeftFootOffset = FMath::FInterpTo(IKLeftFootOffset, GetIKOffsetForASocket(LeftFootSocketName), DeltaSeconds, IKInterpSpeed);
	IKRightFootOffset = FMath::FInterpTo(IKRightFootOffset, GetIKOffsetForASocket(RightFootSocketName), DeltaSeconds, IKInterpSpeed);
	IKBodyOffset = FMath::FInterpTo(IKBodyOffset, GetIKOffsetForBody(), DeltaSeconds, IKInterpSpeed);

	TraceLineOfSight();
}

void AHWBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentFallApex = GetActorLocation();

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AHWBaseCharacter::OnPlayerCapsuleHit);
	CharacterAttributesComponent->OnAttributeChangedDelegate.AddUObject(this, &AHWBaseCharacter::OnAttributeChanged);

	InitializeHealthProgress();
}

void AHWBaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OnInteractiveObjectFound.IsBound())
	{
		OnInteractiveObjectFound.Unbind();
	}
	Super::EndPlay(EndPlayReason);
}

void AHWBaseCharacter::LieDown(bool bClientSimulation)
{
	if (IsValid(BaseCharacterMovementComponent) && CanBeProne())
	{
		BaseCharacterMovementComponent->bWantsToBeProne = true;
		BaseCharacterMovementComponent->bWantsToCrouch = 0;
	}
}

void AHWBaseCharacter::StandUp(bool bClientSimulation)
{
	if (IsValid(BaseCharacterMovementComponent) && CanCrouch())
	{
		BaseCharacterMovementComponent->bWantsToBeProne = false;
		BaseCharacterMovementComponent->bWantsToCrouch = 1;
	}
}

bool AHWBaseCharacter::CanBeProne() const
{
	return !IsProne() && BaseCharacterMovementComponent && GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics()
	&& !bIsSliding;
}

bool AHWBaseCharacter::IsProne() const
{
	return BaseCharacterMovementComponent->IsProne();
}

FRotator AHWBaseCharacter::GetAimOffset()
{
	FVector AimDirectionWorld = GetBaseAimRotation().Vector();
	FVector AimDirectionLocal = GetTransform().InverseTransformVectorNoScale(AimDirectionWorld);
	return AimDirectionLocal.ToOrientationRotator();
}

UHWBaseCharacterMovementComponent* AHWBaseCharacter::GetBaseCharacterMovement() const
{
	return BaseCharacterMovementComponent;
}

const UCharacterEquipmentComponent* AHWBaseCharacter::GetEquipmentComponent() const
{
	return CharacterEquipmentComponent;
}

UCharacterEquipmentComponent* AHWBaseCharacter::GetEquipmentComponent_Mutable() const
{
	return CharacterEquipmentComponent;
}

const UCharacterAttributesComponent* AHWBaseCharacter::GetCharacterAttributes() const
{
	return CharacterAttributesComponent;
}

UCharacterAttributesComponent* AHWBaseCharacter::GetCharacterAttributes_Mutable()
{
	return CharacterAttributesComponent;
}

UCharacterInventoryComponent* AHWBaseCharacter::GetCharacterInventory() const
{
	return CharacterInventoryComponent;
}

FGenericTeamId AHWBaseCharacter::GetGenericTeamId() const
{
	return FGenericTeamId(StaticCast<uint8>(Team));
}

void AHWBaseCharacter::OnRep_IsMantling(bool bWasMantling)
{
	if (GetLocalRole() == ROLE_SimulatedProxy && !bWasMantling && bIsMantling)
	{
		Mantle(true);
	}
}

void AHWBaseCharacter::OnRep_IsSliding()
{
	if (IsValid(BaseCharacterMovementComponent))
	{
		if (bIsSliding)
		{
			bIsSlideRequested = true;
			BaseCharacterMovementComponent->StartSliding(true);
		}
		else
		{
			bIsSlideRequested = false;
			BaseCharacterMovementComponent->StopSliding(true);	
		}
		BaseCharacterMovementComponent->bNetworkUpdateReceived = true;
	}
}

void AHWBaseCharacter::OnSprintStart_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("AHWBaseCharacter::OnSprintStart_Implementation()"));
}

void AHWBaseCharacter::OnSprintStop_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("AHWBaseCharacter::OnSprintStop_Implementation()"));
}

bool AHWBaseCharacter::CanSprint()
{
	return !BaseCharacterMovementComponent->IsOutOfStamina() && !BaseCharacterMovementComponent->IsProne() && !BaseCharacterMovementComponent->IsMantling()
	&& !BaseCharacterMovementComponent->IsOnLadder() && !BaseCharacterMovementComponent->IsOnZipline() && ForwardAxis > 0.f;
}

bool AHWBaseCharacter::CanJumpInternal_Implementation() const
{
	return !BaseCharacterMovementComponent->IsMantling() && !BaseCharacterMovementComponent->IsOutOfStamina()
		&& !bIsSliding && Super::CanJumpInternal_Implementation();
}

bool AHWBaseCharacter::CanMantle() const
{
	return !GetBaseCharacterMovement()->IsOnLadder() && !GetBaseCharacterMovement()->IsMantling()
	&& !bIsSliding;
}

bool AHWBaseCharacter::CanInteractWithLadder() const
{
	return !bIsSliding;
}

bool AHWBaseCharacter::CanCrouch() const
{
	return !bIsSliding && !GetBaseCharacterMovement()->IsSprinting() && Super::CanCrouch();
}

void AHWBaseCharacter::OnStartAimInternal()
{
	if (OnAimingStateChanged.IsBound())
	{
		OnAimingStateChanged.Broadcast(true);
	}
}

void AHWBaseCharacter::OnStopAimInternal()
{
	if (OnAimingStateChanged.IsBound())
	{
		OnAimingStateChanged.Broadcast(false);
	}
}

void AHWBaseCharacter::OnDeath()
{
	if (!IsValid(OnDeathAnimMontage) || PlayAnimMontage(OnDeathAnimMontage) <= 0.f)
	{
		EnableRagdoll();
	}
	if (IsValid(HealthBarProgressComponent))
	{
		HealthBarProgressComponent->SetVisibility(false);
	}
	GetCharacterMovement()->DisableMovement();
}

void AHWBaseCharacter::OnOutOfStamina(bool bIsOutOfStamina)
{
	GetBaseCharacterMovement()->SetOutOfStamina(bIsOutOfStamina);
}

void AHWBaseCharacter::OnHealthChanged(float AttributePercent)
{
	if (IsValid(HealthBarProgressComponent))
	{
		UUserWidget* Widget = HealthBarProgressComponent->GetUserWidgetObject();
		if (IsValid(Widget) && Widget->IsA<UHWAttributeProgressBar>())
		{
			StaticCast<UHWAttributeProgressBar*>(Widget)->SetProgressPercentage(AttributePercent);
		}
	}
	
	if (FMath::IsNearlyZero(AttributePercent))
	{
		OnDeath();
	}
}

void AHWBaseCharacter::OnAttributeChanged(EAttributes Attribute, float AttributePercent)
{
	switch (Attribute)
	{
	case EAttributes::Health:
		OnHealthChanged(AttributePercent);
		break;
	case EAttributes::Stamina:
		if (FMath::IsNearlyZero(AttributePercent) || FMath::IsNearlyZero(AttributePercent - 1.f))
		{
			OnOutOfStamina(StaticCast<bool>(1.f - AttributePercent));
		}
		break;
	case EAttributes::Oxygen:
	default:
		break;
	}
}

void AHWBaseCharacter::InitializeHealthProgress()
{
	UHWAttributeProgressBar* Widget = Cast<UHWAttributeProgressBar>(HealthBarProgressComponent->GetUserWidgetObject());
	if (!IsValid(Widget))
	{
		HealthBarProgressComponent->SetVisibility(false);
		return;
	}

	if (IsPlayerControlled() && IsLocallyControlled())
	{
		HealthBarProgressComponent->SetVisibility(false);
	}

	Widget->SetProgressPercentage(CharacterAttributesComponent->GetHealthPercent());
}

void AHWBaseCharacter::OnPlayerCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	FVector HitNormal = Hit.ImpactNormal;

	if (GetBaseCharacterMovement()->IsWallRunning()) 
	{
		return;
	}

	if (!GetCharacterMovement()->IsFalling())
	{
		return;
	}

	if (!IsSurfaceWallRunable(HitNormal)) 
	{
		return;
	}
	EWallRunSide Side = EWallRunSide::None;
	FVector Direction = FVector::ZeroVector;
	GetWallRunSideAndDirection(HitNormal, Side, Direction);

	if (!AreRequiredKeysDown(Side)) 
	{
		return;
	}

	GetBaseCharacterMovement()->StartWallRun(Side, Direction);
}

void AHWBaseCharacter::OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) && IsValid(MantlingSettings.MantlingMontage)) 
	{
		AnimInstance->Montage_Play(MantlingSettings.MantlingMontage, 1.f, EMontagePlayReturnType::Duration, MantlingAnimationStartTime);
	}
}

void AHWBaseCharacter::TraceLineOfSight()
{
	if (!IsPlayerControlled())
	{
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	AController* CharacterController = GetController();
	if (!IsValid(CharacterController))
	{
		return;
	}
	
	CharacterController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	FVector ViewDirection = ViewRotation.Vector();
	FVector TraceEnd = ViewLocation + ViewDirection * LineOfSightDistance;
	FHitResult HitResult;
	
	GetWorld()->LineTraceSingleByChannel(HitResult, ViewLocation, TraceEnd, ECC_Visibility);
	if (LineOfSightObject.GetObject() != HitResult.Actor)
	{
		FName ActionName = NAME_None;
		LineOfSightObject = HitResult.Actor.Get();
		if (LineOfSightObject.GetInterface())
		{
			ActionName = LineOfSightObject->GetActionEventName();
		}
		OnInteractiveObjectFound.ExecuteIfBound(ActionName);
	}
}

void AHWBaseCharacter::Server_DestroyActor_Implementation(AActor* Actor) const
{
	Actor->Destroy();
}

template <typename T>
const T* AHWBaseCharacter::GetAvailable() const
{
	const T* Result = nullptr;
	for (const AInteractiveActor* InteractiveActor : AvailableInteractiveActors)
	{
		if (InteractiveActor->IsA<T>())
		{
			Result = StaticCast<const T*>(InteractiveActor);
			break;
		}
	}
	return Result;
}

bool AHWBaseCharacter::IsSurfaceWallRunable(const FVector& Normal) const
{
	float Tolerance = -0.005f;
	FHitResult Hit;
	FVector TraceStart = GetActorLocation();
	FVector TraceEnd = TraceStart - Normal * GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	// Check for wall runable trace
	bool bIsWallRunable = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WallRunable);
	// Check that character is almost parallel to plane
	bIsWallRunable &= FMath::Abs(FVector::DotProduct(Normal, GetActorForwardVector())) < 0.5;
	// Check that surface is not walkable and has not downward direction
	bIsWallRunable &= Normal.Z < GetCharacterMovement()->GetWalkableFloorZ() && Normal.Z > Tolerance;
	return bIsWallRunable;
}

void AHWBaseCharacter::TryChangeSprintState()
{
	if (bIsSprintRequested && !BaseCharacterMovementComponent->IsSprinting() && CanSprint())
	{
		GetBaseCharacterMovement()->StartSprint();
		OnSprintStart();
	}

	if (!bIsSprintRequested && GetBaseCharacterMovement()->IsSprinting())
	{
		BaseCharacterMovementComponent->StopSprint();
		OnSprintStop();
	}
}

void AHWBaseCharacter::TryChangeSlideState()
{
	if (GetBaseCharacterMovement()->IsSprinting() && bIsSlideRequested && !bIsSliding)
	{
		GetBaseCharacterMovement()->StartSliding();
	}
}

float AHWBaseCharacter::GetIKOffsetForASocket(const FName& SocketName)
{
	float Result = 0.f;
	
	float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float CharacterFloor = GetActorLocation().Z - HalfHeight;
	const FRotator Orientation(GetMesh()->GetSocketRotation(SocketName));
	const FVector SocketLocation = GetMesh()->GetSocketLocation(SocketName);
	const FVector TraceStart(SocketLocation.X, SocketLocation.Y, GetActorLocation().Z);
	const FVector TraceEnd = TraceStart - (HalfHeight + IKTraceExtendDistance) * FVector::UpVector;
	const FVector HalfSize(0.f, 15.f, 6.f);

	FHitResult HitResult;
	const ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	if (UKismetSystemLibrary::BoxTraceSingle(GetWorld(), TraceStart, TraceEnd, HalfSize, Orientation, TraceType, true, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, HitResult, true))
	{
		Result = CharacterFloor - HitResult.Location.Z;
	}
	
	return Result;
}

float AHWBaseCharacter::GetIKOffsetForBody() const
{
	if (GetCharacterMovement()->IsCrouching())
	{
		return 0.f;
	}
	return -FMath::Abs(IKLeftFootOffset - IKRightFootOffset);
}

const FMantlingSettings& AHWBaseCharacter::GetMantlingSettings(float LedgeHeight) const
{
	return LedgeHeight > LowMantleMaxHeight ? HighMantleSettings : LowMantleSettings;
}

void AHWBaseCharacter::EnableRagdoll()
{
	GetMesh()->SetCollisionProfileName(CollisionProfileNoRagdoll);
	GetMesh()->SetSimulatePhysics(true);
}

void AHWBaseCharacter::OnRep_Aiming(bool bIsAiming_Old)
{
	if (bIsAiming && !bIsAiming_Old)
	{
		StartAimInternal();
	}
	else if (!bIsAiming && bIsAiming_Old)
	{
		StopAimInternal();
	}
}

void AHWBaseCharacter::Server_StartAim_Implementation()
{
	StartAimInternal();
}

void AHWBaseCharacter::StartAimInternal()
{
	ARangeWeaponItem* CurrentRangeWeapon = GetEquipmentComponent()->GetCurrentRangeWeapon();
	if (!IsValid(CurrentRangeWeapon))
	{
		return;
	}
	bIsAiming = true;
	CurrentAimingMovementSpeed = CurrentRangeWeapon->GetAimMaxMoveSpeed();
	CurrentRangeWeapon->StartAim();
	OnStartAim();
}

void AHWBaseCharacter::Server_StopAim_Implementation()
{
	StopAimInternal();
}

void AHWBaseCharacter::StopAimInternal()
{
	ARangeWeaponItem* CurrentRangeWeapon = GetEquipmentComponent()->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StopAim();
	}
	
	bIsAiming = false;
	CurrentAimingMovementSpeed = 0.f;
	OnStoptAim();
}
