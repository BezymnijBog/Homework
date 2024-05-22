// Fill out your copyright notice in the Description page of Project Settings.


#include "HWBaseCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "GameFramework/Character.h"
#include "Homework/Actors/Interactive/Environment/Ladder.h"
#include "Homework/Actors/Interactive/Environment/Zipline.h"
#include "Homework/Characters/HWBaseCharacter.h"

float UHWBaseCharacterMovementComponent::GetMaxSpeed() const
{
	float Result = Super::GetMaxSpeed();
	if (bIsSprinting)
	{
		Result = SprintSpeed;
	}
	else if (bIsOutOfStamina)
	{
		Result = OutOfStaminaMaxSpeed;
	}
	else if (bIsProne)
	{
		Result = MaxProneSpeed;
	}
	else if (IsOnLadder())
	{
		Result = MaxLadderSpeed;
	}
	else if (GetBaseCharacterOwner()->IsAiming())
	{
		Result = GetBaseCharacterOwner()->GetAimingMoveSpeed();
	}
	return Result;
}

FNetworkPredictionData_Client* UHWBaseCharacterMovementComponent::GetPredictionData_Client() const
{
	if (!ClientPredictionData)
	{
		UHWBaseCharacterMovementComponent* MutableThis = const_cast<UHWBaseCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Character_HW(*this);
	}
	return ClientPredictionData;
}

void UHWBaseCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	/*
	*	FLAG_Reserved_1		= 0x04,	// Reserved for future use
	*	FLAG_Reserved_2		= 0x08,	// Reserved for future use
	*	// Remaining bit masks are available for custom flags.
	*	FLAG_Custom_0		= 0x10, - Sprinting flag
	*	FLAG_Custom_1		= 0x20, - Mantling flag
	*	FLAG_Custom_2		= 0x40, - Sliding flag
	*	FLAG_Custom_3		= 0x80
	*
	**/
	bool bWasMantling = GetBaseCharacterOwner()->bIsMantling;
	bool bWasSliding = GetBaseCharacterOwner()->bIsSliding;
	bIsSprinting = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	bool bIsMantling = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
	bool bIsSliding = (Flags & FSavedMove_Character::FLAG_Custom_2) != 0;

	if (GetBaseCharacterOwner()->GetLocalRole() == ROLE_Authority)
	{
		if (!bWasMantling && bIsMantling)
		{
			GetBaseCharacterOwner()->Mantle(true);
		}
		if (!bWasSliding && bIsSliding)
		{
			StartSliding();
		}
	}
}

void UHWBaseCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	// no need to call base method here
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		const bool bIsCrouching = IsCrouching();
		if (bIsCrouching && (!bWantsToCrouch && !bWantsToBeProne || !CanCrouchInCurrentState()))
		{
			UnCrouch(false);
		}
		else if (!bIsCrouching && !bIsProne && bWantsToCrouch && CanCrouchInCurrentState())
		{
			Crouch(false);
		}
		else if (bIsProne && (!bWantsToBeProne && !bWantsToCrouch || !CanCrouchInCurrentState()))
		{
			StandUp(false);
		}
		else if (bIsProne && bWantsToCrouch && !bWantsToBeProne)
		{
			StandUp(false);
			Crouch(false);
		}
		else if (bIsCrouching && bWantsToBeProne && CanBeProneInCurrentState())
		{
			UnCrouch(false);
			LieDown(false);
		}
	}
}

void UHWBaseCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	if (bForceRotation)
	{
		FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); // Normalized
		CurrentRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): CurrentRotation"));

		FRotator DeltaRot = GetDeltaRotation(DeltaTime);
		DeltaRot.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): GetDeltaRotation"));
		// Accumulate a desired new rotation.
		const float AngleTolerance = 1e-3f;

		if (!CurrentRotation.Equals(ForceTargetRotation, AngleTolerance))
		{
			FRotator DesiredRotation = ForceTargetRotation;
			// PITCH
			if (!FMath::IsNearlyEqual(CurrentRotation.Pitch, DesiredRotation.Pitch, AngleTolerance))
			{
				DesiredRotation.Pitch = FMath::FixedTurn(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaRot.Pitch);
			}

			// YAW
			if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, AngleTolerance))
			{
				DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaRot.Yaw);
			}

			// ROLL
			if (!FMath::IsNearlyEqual(CurrentRotation.Roll, DesiredRotation.Roll, AngleTolerance))
			{
				DesiredRotation.Roll = FMath::FixedTurn(CurrentRotation.Roll, DesiredRotation.Roll, DeltaRot.Roll);
			}

			// Set the new rotation.
			DesiredRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): DesiredRotation"));
			MoveUpdatedComponent(FVector::ZeroVector, DesiredRotation, /*bSweep*/ false);
		}
		else
		{
			ForceTargetRotation = FRotator::ZeroRotator;
			bForceRotation = false;
		}
		return;

	}
	if (IsOnLadder())
	{
		return;
	}
	Super::PhysicsRotation(DeltaTime);
}

void UHWBaseCharacterMovementComponent::SetOutOfStamina(bool bIsOutOfStamina_In)
{
	bIsOutOfStamina = bIsOutOfStamina_In;
	if (bIsOutOfStamina)
	{
		StopSprint();
	}
}

void UHWBaseCharacterMovementComponent::StartSprint()
{
	bIsSprinting = true;
	bForceMaxAccel = 1;
}

void UHWBaseCharacterMovementComponent::StopSprint()
{
	bIsSprinting = false;
	bForceMaxAccel = 0;
}

void UHWBaseCharacterMovementComponent::StartMantle(const FMantlingMovementParameters& MantlingParameters)
{
	CurrentMantlingParameters = MantlingParameters;
	SetMovementMode(MOVE_Custom, StaticCast<uint8>(ECustomMovementMode::CMOVE_Mantling));
}

void UHWBaseCharacterMovementComponent::EndMantle()
{
	GetBaseCharacterOwner()->bIsMantling = false;
	SetMovementMode(MOVE_Walking);
}

bool UHWBaseCharacterMovementComponent::IsMantling() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == StaticCast<uint8>(ECustomMovementMode::CMOVE_Mantling);
}

void UHWBaseCharacterMovementComponent::StartSliding(bool bIsClientSimulation)
{
	if (!HasValidData() || (!CanSlideInCurrentState() && !bIsClientSimulation))
	{
		return;
	}

	//bool bHasAuthority = GetBaseCharacterOwner()->HasAuthority();
	bOrientRotationToMovement = 0;
	CustomMoveDirection = Velocity.GetSafeNormal();
	if (GetBaseCharacterOwner()->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == SlideCapsuleHalfHeight)
	{
		if (!bIsClientSimulation)
		{
			GetBaseCharacterOwner()->bIsSliding = true;
		}
		GetBaseCharacterOwner()->OnStartSliding(0.f, 0.f);
		return;
	}

	if (bIsClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		AHWBaseCharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<AHWBaseCharacter>();
		GetBaseCharacterOwner()->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
		bShrinkProxyCapsule = true;
	}

	// Change collision size to slide dimensions
	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	// Height is not allowed to be smaller than radius.
	const float ClampedSlideHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, SlideCapsuleHalfHeight);
	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, ClampedSlideHalfHeight);
	// Get half-height adjust relative to base character
	float HalfHeightAdjust = (OldUnscaledHalfHeight - ClampedSlideHalfHeight);
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	if (!bIsClientSimulation)
	{
		//GetWorld()->GetTimerManager().SetTimer(CustomMovementTimer, [this]() { StopSliding(false); }, SlideTime, false);

		if (bCrouchMaintainsBaseLocation)
		{
			// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
			UpdatedComponent->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
		}
		GetBaseCharacterOwner()->bIsSliding = true;
	}

	const float MeshAdjust = ScaledHalfHeightAdjust;
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	HalfHeightAdjust = (DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - ClampedSlideHalfHeight);
	ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	AdjustProxyCapsuleSize();
	GetBaseCharacterOwner()->OnStartSliding(HalfHeightAdjust, ScaledHalfHeightAdjust);

	if ((bIsClientSimulation && GetBaseCharacterOwner()->GetLocalRole() == ROLE_SimulatedProxy) || (IsNetMode(NM_ListenServer) && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy))
	{
		FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
		if (ClientData)
		{
			ClientData->MeshTranslationOffset -= FVector(0.f, 0.f, MeshAdjust);
			ClientData->OriginalMeshTranslationOffset = ClientData->MeshTranslationOffset;
		}
	}
}

void UHWBaseCharacterMovementComponent::StopSliding(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	AHWBaseCharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<AHWBaseCharacter>();

	bOrientRotationToMovement = 1;
	CustomMoveDirection = FVector::ZeroVector;
	if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight())
	{
		if (!bClientSimulation)
		{
			GetBaseCharacterOwner()->bIsSliding = false;
		}
		GetBaseCharacterOwner()->OnEndSliding(0.f, 0.f);
		return;
	}

	const float CurrentSlideHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float HalfHeightAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - OldUnscaledHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	if (!bClientSimulation)
	{
		GetWorld()->GetTimerManager().ClearTimer(CustomMovementTimer);
		// Try to stay in place and see if the larger capsule fits. We use a slightly taller capsule to avoid penetration.
		const UWorld* MyWorld = GetWorld();
		const float SweepInflation = KINDA_SMALL_NUMBER * 10.f;
		FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(SlideTrace), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(CapsuleParams, ResponseParam);

		// Compensate for the difference between current capsule size and standing size
		const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust); // Shrink by negative amount, so actually grow it.
		const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
		bool bEncroached = true;

		if (!bCrouchMaintainsBaseLocation)
		{
			bEncroached = MyWorld->OverlapBlockingTestByChannel(PawnLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				// Try adjusting capsule position to see if we can avoid encroachment.
				if (ScaledHalfHeightAdjust > 0.f)
				{
					// Shrink to a short capsule, sweep down to base to find where that would hit something, and then try to stand up from there.
					float PawnRadius, PawnHalfHeight;
					CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
					const float ShrinkHalfHeight = PawnHalfHeight - PawnRadius;
					const float TraceDist = PawnHalfHeight - ShrinkHalfHeight;
					const FVector Down = FVector(0.f, 0.f, -TraceDist);

					FHitResult Hit(1.f);
					const FCollisionShape ShortCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, ShrinkHalfHeight);
					const bool bBlockingHit = MyWorld->SweepSingleByChannel(Hit, PawnLocation, PawnLocation + Down, FQuat::Identity, CollisionChannel, ShortCapsuleShape, CapsuleParams);
					if (Hit.bStartPenetrating)
					{
						bEncroached = true;
					}
					else
					{
						// Compute where the base of the sweep ended up, and see if we can stand there
						const float DistanceToBase = (Hit.Time * TraceDist) + ShortCapsuleShape.Capsule.HalfHeight;
						const FVector NewLoc = FVector(PawnLocation.X, PawnLocation.Y, PawnLocation.Z - DistanceToBase + StandingCapsuleShape.Capsule.HalfHeight + SweepInflation + MIN_FLOOR_DIST / 2.f);
						bEncroached = MyWorld->OverlapBlockingTestByChannel(NewLoc, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
						if (!bEncroached)
						{
							// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
							UpdatedComponent->MoveComponent(NewLoc - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
						}
					}
				}
			}
		}
		else
		{
			// Expand while keeping base location the same.
			FVector StandingLocation = PawnLocation + FVector(0.f, 0.f, StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentSlideHalfHeight);
			bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				if (IsMovingOnGround())
				{
					// Something might be just barely overhead, try moving down closer to the floor to avoid it.
					const float MinFloorDist = KINDA_SMALL_NUMBER * 10.f;
					if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
					{
						StandingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
						bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
					}
				}
			}

			if (!bEncroached)
			{
				// Commit the change in location.
				UpdatedComponent->MoveComponent(StandingLocation - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
				bForceNextFloorCheck = true;
			}
		}

		// If still encroached then abort.
		if (bEncroached)
		{
			return;
		}

		GetBaseCharacterOwner()->bIsSliding = false;
	}
	else
	{
		bShrinkProxyCapsule = true;
	}

	// Now call SetCapsuleSize() to cause touch/untouch events and actually grow the capsule
	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), true);
	const float MeshAdjust = ScaledHalfHeightAdjust;
	AdjustProxyCapsuleSize();
	GetBaseCharacterOwner()->OnEndSliding(HalfHeightAdjust, ScaledHalfHeightAdjust);

	// Don't smooth this change in mesh position
	if ((bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy) || (IsNetMode(NM_ListenServer) && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy))
	{
		FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
		if (ClientData)
		{
			ClientData->MeshTranslationOffset += FVector(0.f, 0.f, MeshAdjust);
			ClientData->OriginalMeshTranslationOffset = ClientData->MeshTranslationOffset;
		}
	}
}

bool UHWBaseCharacterMovementComponent::CanSlideInCurrentState() const
{
	return CanCrouchInCurrentState() && bIsSprinting;
}

void UHWBaseCharacterMovementComponent::LieDown(bool bClientSimulation)
{
	// meaningful copy and paste from UCharacterMovementComponent::Crouch()
	if (!HasValidData())
	{
		return;
	}

	if (!bClientSimulation && !CanBeProneInCurrentState())
	{
		return;
	}

	// See if collision is already at desired size.
	if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == ProneCapsuleHalfHeight)
	{
		if (!bClientSimulation)
		{
			bIsProne = true;
		}
		GetBaseCharacterOwner()->OnStartProne(0.f, 0.f);
		return;
	}

	if (bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		// restore collision size before being prone
		ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
		bShrinkProxyCapsule = true;
	}

	// Change collision size to prone dimensions
	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	// Height is not allowed to be smaller than radius.
	const float ClampedProneHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, ProneCapsuleHalfHeight);
	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, ClampedProneHalfHeight);
	float HalfHeightAdjust = (OldUnscaledHalfHeight - ClampedProneHalfHeight);
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	if (!bClientSimulation)
	{
		// Prone to a larger height? (this is rare)
		if (ClampedProneHalfHeight > OldUnscaledHalfHeight)
		{
			FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(ProneTrace), false, CharacterOwner);
			FCollisionResponseParams ResponseParam;
			InitCollisionParams(CapsuleParams, ResponseParam);
			const bool bEncroached = GetWorld()->OverlapBlockingTestByChannel(UpdatedComponent->GetComponentLocation() - FVector(0.f, 0.f, ScaledHalfHeightAdjust), FQuat::Identity,
				UpdatedComponent->GetCollisionObjectType(), GetPawnCapsuleCollisionShape(SHRINK_None), CapsuleParams, ResponseParam);

			// If encroached, cancel
			if (bEncroached)
			{
				CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, OldUnscaledHalfHeight);
				return;
			}
		}

		if (bCrouchMaintainsBaseLocation)
		{
			// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
			UpdatedComponent->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
		}

		bIsProne = true;
	}

	bForceNextFloorCheck = true;

	// OnStartProne takes the change from the Default size, not the current one (though they are usually the same).
	const float MeshAdjust = ScaledHalfHeightAdjust;
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	HalfHeightAdjust = (DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - ClampedProneHalfHeight);
	ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	AdjustProxyCapsuleSize();
	GetBaseCharacterOwner()->OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);

	// Don't smooth this change in mesh position
	if ((bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy) || (IsNetMode(NM_ListenServer) && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy))
	{
		FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
		if (ClientData)
		{
			ClientData->MeshTranslationOffset -= FVector(0.f, 0.f, MeshAdjust);
			ClientData->OriginalMeshTranslationOffset = ClientData->MeshTranslationOffset;
		}
	}
}

void UHWBaseCharacterMovementComponent::StandUp(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight())
	{
		if (!bClientSimulation)
		{
			bIsProne = false;
		}
		GetBaseCharacterOwner()->OnEndProne(0.f, 0.f);
		return;
	}

	const float CurrentProneHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float HalfHeightAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - OldUnscaledHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	// Grow to unprone size.
	check(CharacterOwner->GetCapsuleComponent());

	if (!bClientSimulation)
	{
		// Try to stay in place and see if the larger capsule fits. We use a slightly taller capsule to avoid penetration.
		const UWorld* MyWorld = GetWorld();
		const float SweepInflation = KINDA_SMALL_NUMBER * 10.f;
		FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(ProneTrace), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(CapsuleParams, ResponseParam);

		// Compensate for the difference between current capsule size and standing size
		const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust); // Shrink by negative amount, so actually grow it.
		const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
		bool bEncroached = true;

		if (!bCrouchMaintainsBaseLocation)
		{
			// Expand in place
			bEncroached = MyWorld->OverlapBlockingTestByChannel(PawnLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				// Try adjusting capsule position to see if we can avoid encroachment.
				if (ScaledHalfHeightAdjust > 0.f)
				{
					// Shrink to a short capsule, sweep down to base to find where that would hit something, and then try to stand up from there.
					float PawnRadius, PawnHalfHeight;
					CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
					const float ShrinkHalfHeight = PawnHalfHeight - PawnRadius;
					const float TraceDist = PawnHalfHeight - ShrinkHalfHeight;
					const FVector Down = FVector(0.f, 0.f, -TraceDist);

					FHitResult Hit(1.f);
					const FCollisionShape ShortCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, ShrinkHalfHeight);
					// bBlockingHit not used, why?
					const bool bBlockingHit = MyWorld->SweepSingleByChannel(Hit, PawnLocation, PawnLocation + Down, FQuat::Identity, CollisionChannel, ShortCapsuleShape, CapsuleParams);
					if (Hit.bStartPenetrating)
					{
						bEncroached = true;
					}
					else
					{
						// Compute where the base of the sweep ended up, and see if we can stand there
						const float DistanceToBase = (Hit.Time * TraceDist) + ShortCapsuleShape.Capsule.HalfHeight;
						const FVector NewLoc = FVector(PawnLocation.X, PawnLocation.Y, PawnLocation.Z - DistanceToBase + StandingCapsuleShape.Capsule.HalfHeight + SweepInflation + MIN_FLOOR_DIST / 2.f);
						bEncroached = MyWorld->OverlapBlockingTestByChannel(NewLoc, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
						if (!bEncroached)
						{
							// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
							UpdatedComponent->MoveComponent(NewLoc - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
						}
					}
				}
			}
		}
		else
		{
			// Expand while keeping base location the same.
			FVector StandingLocation = PawnLocation + FVector(0.f, 0.f, StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentProneHalfHeight);
			bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				if (IsMovingOnGround())
				{
					// Something might be just barely overhead, try moving down closer to the floor to avoid it.
					const float MinFloorDist = KINDA_SMALL_NUMBER * 10.f;
					if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
					{
						StandingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
						bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
					}
				}
			}

			if (!bEncroached)
			{
				// Commit the change in location.
				UpdatedComponent->MoveComponent(StandingLocation - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
				bForceNextFloorCheck = true;
			}
		}

		// If still encroached then abort.
		if (bEncroached)
		{
			return;
		}

		bIsProne = false;
	}
	else
	{
		bShrinkProxyCapsule = true;
	}

	// Now call SetCapsuleSize() to cause touch/untouch events and actually grow the capsule
	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), true);

	const float MeshAdjust = ScaledHalfHeightAdjust;
	AdjustProxyCapsuleSize();
	GetBaseCharacterOwner()->OnEndProne(HalfHeightAdjust, ScaledHalfHeightAdjust);

	// Don't smooth this change in mesh position
	if ((bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy) || (IsNetMode(NM_ListenServer) && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy))
	{
		FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
		if (ClientData)
		{
			ClientData->MeshTranslationOffset += FVector(0.f, 0.f, MeshAdjust);
			ClientData->OriginalMeshTranslationOffset = ClientData->MeshTranslationOffset;
		}
	}
}

bool UHWBaseCharacterMovementComponent::CanBeProneInCurrentState() const
{
	// If character can crouch, so it can be prone
	return CanCrouchInCurrentState();
}

float UHWBaseCharacterMovementComponent::GetActorToCurrentLadderProjection(const FVector& Location) const
{
	checkf(IsValid(CurrentLadder), TEXT("UHWBaseCharacterMovementComponent::GetCharacterToCurrentLadderProjection CurrentLadder is NULL"));
	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	FVector LadderToCharacterDist = Location - CurrentLadder->GetActorLocation();
	return FVector::DotProduct(LadderUpVector, LadderToCharacterDist);
}

float UHWBaseCharacterMovementComponent::GetLadderSpeedRatio() const
{
	checkf(IsValid(CurrentLadder), TEXT("UHWBaseCharacterMovementComponent::GetLadderSpeedRatio(): CurrentLadder is nullptr"));
	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	return FVector::DotProduct(LadderUpVector, Velocity) / MaxLadderSpeed;
}

void UHWBaseCharacterMovementComponent::AttachToLadder(const ALadder* Ladder)
{
	CurrentLadder = Ladder;
	CharacterOwner->bUseControllerRotationYaw = false;

	FRotator TargetOrientRotation = CurrentLadder->GetActorForwardVector().ToOrientationRotator();
	TargetOrientRotation.Yaw += 180.f;

	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	FVector LadderForwardVector = CurrentLadder->GetActorForwardVector();
	float ActorToLadderProjection = GetActorToCurrentLadderProjection(GetActorLocation());

	FVector NewCharacterLocation = CurrentLadder->GetActorLocation() + ActorToLadderProjection * LadderUpVector + LadderToCharacterOffset * LadderForwardVector;
	if (CurrentLadder->IsOnTop())
	{
		NewCharacterLocation = CurrentLadder->GetAttachFromTopAnimMontageStartingLocation();
	}

	GetOwner()->SetActorLocation(NewCharacterLocation);
	GetOwner()->SetActorRotation(TargetOrientRotation);

	SetMovementMode(MOVE_Custom, StaticCast<uint8>(ECustomMovementMode::CMOVE_Ladder));
}

void UHWBaseCharacterMovementComponent::DetachFromLadder(EDetachFromLadderMethod DetachFromLadderMethod)
{
	switch (DetachFromLadderMethod)
	{
	case EDetachFromLadderMethod::ReachingTheBottom:
	{
		SetMovementMode(MOVE_Walking);
		break;
	}
	case EDetachFromLadderMethod::ReachingTheTop:
	{
		GetBaseCharacterOwner()->Mantle(true);
		break;
	}
	case EDetachFromLadderMethod::JumpOff:
	{
		FVector JumpDirection = CurrentLadder->GetActorForwardVector();
		SetMovementMode(MOVE_Falling);
		FVector JumpVelocity = JumpDirection * JumpOffFromLadderSpeed;
		ForceTargetRotation = JumpDirection.ToOrientationRotator();
		bForceRotation = true;
		Launch(JumpVelocity);
		break;
	}
	case EDetachFromLadderMethod::Fall:
	default:
	{
		SetMovementMode(MOVE_Falling);
		break;
	}
	}
	CharacterOwner->bUseControllerRotationYaw = true;
}

bool UHWBaseCharacterMovementComponent::IsOnLadder() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == StaticCast<uint8>(ECustomMovementMode::CMOVE_Ladder);
}

const ALadder* UHWBaseCharacterMovementComponent::GetCurrentLadder() const
{
	return CurrentLadder;
}

void UHWBaseCharacterMovementComponent::AttachToZipline(const AZipline* Zipline)
{
	CharacterOwner->bUseControllerRotationYaw = false;
	CurrentZipline = Zipline;
	CustomMoveDirection = Zipline->GetMoveDirection();
	// calc rotation
	FVector ZiplineDirectionProjection(CustomMoveDirection.X, CustomMoveDirection.Y, 0.f);
	ZiplineDirectionProjection.Normalize();
	FRotator NewCharacterRotation = ZiplineDirectionProjection.ToOrientationRotator();
	// calc location
	float VerticalOffset = GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector GrapplePoint = Zipline->GetClosestCablePoint(GetOwner()->GetActorLocation());
	FVector NewCharacterLocation(GrapplePoint.X, GrapplePoint.Y, GrapplePoint.Z - VerticalOffset);

	GetOwner()->SetActorLocation(NewCharacterLocation);
	GetOwner()->SetActorRotation(NewCharacterRotation);

	SetMovementMode(MOVE_Custom, StaticCast<uint8>(ECustomMovementMode::CMOVE_Zipline));
}

void UHWBaseCharacterMovementComponent::DetachFromZipline(bool bIsJumpedOff)
{
	FVector CurrentVelocity = CustomMoveDirection * ZiplineSpeed;
	if (bIsJumpedOff)
	{
		FVector JumpDirection = FVector::UpVector;
		SetMovementMode(MOVE_Falling);
		FVector JumpVelocity = JumpDirection * JumpOffFromZiplineSpeed;
		Launch(JumpVelocity + CurrentVelocity);
	}
	else
	{
		SetMovementMode(MOVE_Falling);
		Launch(CurrentVelocity);
	}
	CharacterOwner->bUseControllerRotationYaw = true;
}

bool UHWBaseCharacterMovementComponent::IsOnZipline() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == StaticCast<uint8>(ECustomMovementMode::CMOVE_Zipline);
}

const AZipline* UHWBaseCharacterMovementComponent::GetCurrentZipline() const
{
	return CurrentZipline;
}

void UHWBaseCharacterMovementComponent::StartWallRun(EWallRunSide InWallRunSide, const FVector& InMoveDirection)
{
	if (WallRunSide == InWallRunSide)
	{
		return;
	}
	SetMovementMode(MOVE_Custom, StaticCast<uint8>(ECustomMovementMode::CMOVE_WallRun));
	WallRunSide = InWallRunSide;
	CustomMoveDirection = InMoveDirection;
	GetOwner()->SetActorRotation(CustomMoveDirection.ToOrientationRotator());

}

void UHWBaseCharacterMovementComponent::EndWallRun()
{
	GetWorld()->GetTimerManager().ClearTimer(WallRunTimer);
	if (MovementMode != MOVE_Falling && PendingLaunchVelocity.IsNearlyZero())
	{
		Launch(CustomMoveDirection * MaxWallRunSpeed);
		SetMovementMode(MOVE_Falling);
	}
}

bool UHWBaseCharacterMovementComponent::IsWallRunning() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == StaticCast<uint8>(ECustomMovementMode::CMOVE_WallRun);
}

EWallRunSide UHWBaseCharacterMovementComponent::GetWallRunSide() const
{
	return WallRunSide;
}

void UHWBaseCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	if (MovementMode == MOVE_Swimming)
	{
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(SwimmingCapsuleRadius, SwimmingCapsuleHalfHeight);
	}
	else if (PreviousMovementMode == MOVE_Swimming)
	{
		ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
		UCapsuleComponent* DefaultCapsule = DefaultCharacter->GetCapsuleComponent();
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCapsule->GetUnscaledCapsuleRadius(), DefaultCapsule->GetUnscaledCapsuleHalfHeight(), true);
	}
	else if (MovementMode == MOVE_Walking)
	{
		WallRunSide = EWallRunSide::None;
	}
	if (MovementMode == MOVE_Custom)
	{
		switch (CustomMovementMode)
		{
		case (uint8)ECustomMovementMode::CMOVE_Mantling:
		{
			GetWorld()->GetTimerManager().SetTimer(CustomMovementTimer, this,
				&UHWBaseCharacterMovementComponent::EndMantle, CurrentMantlingParameters.Duration, false);
			break;
		}
		case (uint8)ECustomMovementMode::CMOVE_WallRun:
		{
			GetWorld()->GetTimerManager().SetTimer(WallRunTimer, this, &UHWBaseCharacterMovementComponent::EndWallRun, MaxWallRunTime);
		}
		default:
			break;
		}
	}
	if (PreviousMovementMode == MOVE_Custom && PreviousMovementMode == StaticCast<uint8>(ECustomMovementMode::CMOVE_Ladder))
	{
		CurrentLadder = nullptr;
	}
}

void UHWBaseCharacterMovementComponent::PhysMantling(float DeltaTime, int32 /*Iterations*/)
{
	const float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(CustomMovementTimer) + CurrentMantlingParameters.StartTime;
	const FVector CurveValue = CurrentMantlingParameters.MantlingCurve->GetVectorValue(ElapsedTime);
	const float PositionAlpha = CurveValue.X;
	const float XYCorrectionAlpha = CurveValue.Y;
	const float ZCorrectionAlpha = CurveValue.Z;

	FVector CorrectedInitialLocation = FMath::Lerp(CurrentMantlingParameters.InitialLocation, CurrentMantlingParameters.InitialAnimationLocation, XYCorrectionAlpha);
	CorrectedInitialLocation.Z = FMath::Lerp(CurrentMantlingParameters.InitialLocation.Z, CurrentMantlingParameters.InitialAnimationLocation.Z, ZCorrectionAlpha);

	const FVector NewLocation = FMath::Lerp(CorrectedInitialLocation, CurrentMantlingParameters.TargetLocation, PositionAlpha);
	const FRotator NewRotation = FMath::Lerp(CurrentMantlingParameters.InitialRotation, CurrentMantlingParameters.TargetRotation, PositionAlpha);

	const FVector Delta = NewLocation - GetActorLocation() + CurrentMantlingParameters.TargetComponent->GetComponentLocation();
	Velocity = Delta / DeltaTime;

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, NewRotation, false, Hit);
}

void UHWBaseCharacterMovementComponent::PhysLadder(float DeltaTime, int32 Iterations)
{
	CalcVelocity(DeltaTime, 1.f, false, LadderBreakingDeceleration);
	FVector Delta = Velocity * DeltaTime;

	if (HasAnimRootMotion())
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), false, Hit);
		return;
	}

	FVector NewPos = GetActorLocation() + Delta;
	float NewPosProjection = GetActorToCurrentLadderProjection(NewPos);
	if (NewPosProjection < MinLadderBottomOffset)
	{
		DetachFromLadder(EDetachFromLadderMethod::ReachingTheBottom);
		return;
	}
	if (NewPosProjection > (CurrentLadder->GetHeight() - MaxLadderHeightTopOffset))
	{
		DetachFromLadder(EDetachFromLadderMethod::ReachingTheTop);
		return;
	}

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, Hit);
}

void UHWBaseCharacterMovementComponent::PhysZipline(float DeltaTime, int32 Iterations)
{
	const FVector Delta = CustomMoveDirection * ZiplineSpeed * DeltaTime;
	if (CurrentZipline->IsDetachNeeded(GetActorLocation()))
	{
		DetachFromZipline();
	}
	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), false, Hit);
}

void UHWBaseCharacterMovementComponent::PhysWallRun(float DeltaTime, int32 Iterations)
{
	if (!GetBaseCharacterOwner()->AreRequiredKeysDown(WallRunSide))
	{
		EndWallRun();
		return;
	}

	FHitResult HitResult;

	FVector LineTraceDirection = WallRunSide == EWallRunSide::Right ? GetOwner()->GetActorRightVector() : -GetOwner()->GetActorRightVector();
	float LineTraceLength = 400.0f;

	FVector StartPosition = GetActorLocation();
	FVector EndPosition = StartPosition + LineTraceLength * LineTraceDirection;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetCharacterOwner());

	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartPosition, EndPosition, ECC_Visibility, QueryParams))
	{
		EWallRunSide Side = EWallRunSide::None;
		FVector Direction = FVector::ZeroVector;
		GetBaseCharacterOwner()->GetWallRunSideAndDirection(HitResult.ImpactNormal, Side, Direction);
		if (Side != WallRunSide)
		{
			EndWallRun();
		}
		else
		{
			FVector DeltaMove = CustomMoveDirection * MaxWallRunSpeed * DeltaTime;
			SafeMoveUpdatedComponent(DeltaMove, Direction.ToOrientationRotator(), false, HitResult);
			CustomMoveDirection = Direction;
			Velocity = CustomMoveDirection * MaxWallRunSpeed;
		}
	}
	else
	{
		EndWallRun();
	}
}

void UHWBaseCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (GetBaseCharacterOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return;
	}

	const ECustomMovementMode MyCustomMovementMode = StaticCast<ECustomMovementMode>(CustomMovementMode);
	switch (MyCustomMovementMode)
	{
	case ECustomMovementMode::CMOVE_Mantling:
	{
		PhysMantling(DeltaTime, Iterations);
		break;
	}
	case ECustomMovementMode::CMOVE_Ladder:
	{
		PhysLadder(DeltaTime, Iterations);
		break;
	}
	case ECustomMovementMode::CMOVE_Zipline:
	{
		PhysZipline(DeltaTime, Iterations);
		break;
	}
	case ECustomMovementMode::CMOVE_WallRun:
	{
		PhysWallRun(DeltaTime, Iterations);
		break;
	}
	default:
		break;
	}
	Super::PhysCustom(DeltaTime, Iterations);
}

void UHWBaseCharacterMovementComponent::PhysWalking(float DeltaTime, int32 Iterations)
{
	if (GetBaseCharacterOwner()->bIsSliding)
	{

		Velocity = CustomMoveDirection * SlideSpeed;
	}
	Super::PhysWalking(DeltaTime, Iterations);
}

AHWBaseCharacter* UHWBaseCharacterMovementComponent::GetBaseCharacterOwner() const
{
	return StaticCast<AHWBaseCharacter*>(GetOwner());
}

bool FSavedMove_HW::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter, float MaxDelta) const
{
	FSavedMove_HW* NewMove = StaticCast<FSavedMove_HW*>(NewMovePtr.Get());

	if (bSavedIsSprinting != NewMove->bSavedIsSprinting
		|| bSavedIsMantling != NewMove->bSavedIsMantling
		|| bSavedIsSlideRequested != NewMove->bSavedIsSlideRequested)
	{
		return false;
	}
	return Super::CanCombineWith(NewMovePtr, InCharacter, MaxDelta);
}

void FSavedMove_HW::Clear()
{
	Super::Clear();
	bSavedIsSprinting = 0;
	bSavedIsMantling = 0;
	bSavedIsSlideRequested = 0;
}

uint8 FSavedMove_HW::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	/*
	*	FLAG_Reserved_1		= 0x04,	// Reserved for future use
	*	FLAG_Reserved_2		= 0x08,	// Reserved for future use
	*	// Remaining bit masks are available for custom flags.
	*	FLAG_Custom_0		= 0x10 - Sprinting flag
	*	FLAG_Custom_1		= 0x20 - Mantling flag
	*	FLAG_Custom_2		= 0x40 - Sliding flag
	*	FLAG_Custom_3		= 0x80
	*
	**/
	if (bSavedIsSprinting)
	{
		Result |= FLAG_Custom_0;
	}
	if (bSavedIsMantling)
	{
		Result &= ~FLAG_JumpPressed;
		Result |= FLAG_Custom_1;
	}
	if (bSavedIsSlideRequested)
	{
		Result &= ~(FLAG_JumpPressed | FLAG_WantsToCrouch | FLAG_Custom_1);
		Result |= FLAG_Custom_2;
	}
	return Result;
}

void FSavedMove_HW::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	AHWBaseCharacter* BaseCharacter = StaticCast<AHWBaseCharacter*>(Character);
	UHWBaseCharacterMovementComponent* MovementComponent = BaseCharacter->GetBaseCharacterMovement();
	MovementComponent->bIsSprinting = bSavedIsSprinting;
	BaseCharacter->bIsSlideRequested = bSavedIsSlideRequested;
}

void FSavedMove_HW::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
	FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	check(C->IsA<AHWBaseCharacter>());
	AHWBaseCharacter* BaseCharacter = StaticCast<AHWBaseCharacter*>(C);
	UHWBaseCharacterMovementComponent* MovementComponent = BaseCharacter->GetBaseCharacterMovement();
	bSavedIsSprinting = MovementComponent->IsSprinting();
	bSavedIsSlideRequested = BaseCharacter->bIsSlideRequested;
	bSavedIsMantling = BaseCharacter->bIsMantling;
}

FNetworkPredictionData_Client_Character_HW::FNetworkPredictionData_Client_Character_HW(
	const UHWBaseCharacterMovementComponent& ClientMovement) : Super(ClientMovement) {}

FSavedMovePtr FNetworkPredictionData_Client_Character_HW::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_HW);
}
