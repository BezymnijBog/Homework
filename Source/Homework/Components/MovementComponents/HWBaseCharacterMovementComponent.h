// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HWBaseCharacterMovementComponent.generated.h"

class AZipline;
class ALadder;

struct FMantlingMovementParameters
{
	FVector InitialLocation = FVector::ZeroVector;
	FRotator InitialRotation = FRotator::ZeroRotator;

	FVector TargetLocation = FVector::ZeroVector;
	FRotator TargetRotation = FRotator::ZeroRotator;

	UPrimitiveComponent* TargetComponent;
	
	FVector InitialAnimationLocation = FVector::ZeroVector;
	
	float Duration = 1.0f;
	float StartTime = 0.0f;

	UCurveVector* MantlingCurve;
};

UENUM(BlueprintType)
enum class ECustomMovementMode : uint8
{
	CMOVE_None = 0 UMETA(DisplayName = "None"),
	CMOVE_Mantling UMETA(DisplayName = "Mantling"),
	CMOVE_Ladder UMETA(DisplayName = "Ladder"),
	CMOVE_Zipline UMETA(DisplayName = "Zipline"),
	CMOVE_WallRun UMETA(DisplayName = "Wall run"),
	CMOVE_Max UMETA(Hidden)
};

UENUM()
enum class EDetachFromLadderMethod : uint8
{
	Fall = 0,
	ReachingTheTop,
	ReachingTheBottom,
	JumpOff
};

UENUM()
enum class EWallRunSide
{
	None = 0,
	Right,
	Left
};

UCLASS()
class HOMEWORK_API UHWBaseCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	friend class FSavedMove_HW;

public:
	bool IsProne() const { return bIsProne; }
	bool IsOutOfStamina() const { return bIsOutOfStamina; }
	FORCEINLINE bool IsSprinting() const { return bIsSprinting; }
	virtual float GetMaxSpeed() const override;

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void PhysicsRotation(float DeltaTime) override;

	void SetOutOfStamina(bool bIsOutOfStamina_In);
	void StartSprint();
	void StopSprint();
	
	void StartMantle(const FMantlingMovementParameters& MantlingParameters);
	void EndMantle();
	bool IsMantling() const;

	void StartSliding(bool bIsClientSimulation = false);
	void StopSliding(bool bClientSimulation = false);
	bool CanSlideInCurrentState() const;

	virtual void LieDown(bool bClientSimulation = false);
	virtual void StandUp(bool bClientSimulation = false);
	
	virtual bool CanBeProneInCurrentState() const;
	float GetActorToCurrentLadderProjection(const FVector& Location) const;
	float GetLadderSpeedRatio() const;
	void AttachToLadder(const ALadder* Ladder);
	void DetachFromLadder(EDetachFromLadderMethod DetachFromLadderMethod = EDetachFromLadderMethod::Fall);
	bool IsOnLadder() const;
	const ALadder* GetCurrentLadder() const;

	void AttachToZipline(const AZipline* Zipline);
	void DetachFromZipline(bool bIsJumpedOff = false);
	bool IsOnZipline() const;
	const AZipline* GetCurrentZipline() const;

	void StartWallRun(EWallRunSide InWallRunSide, const FVector& InMoveDirection);
	void EndWallRun();
	bool IsWallRunning() const;
	EWallRunSide GetWallRunSide() const;
	
	UPROPERTY(Category = "Character Movement (General Settings)", VisibleInstanceOnly, BlueprintReadOnly)
	bool bWantsToBeProne = false;
	
protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	void PhysMantling(float DeltaTime, int32 Iterations);
	void PhysLadder(float DeltaTime, int32 Iterations);
	void PhysZipline(float DeltaTime, int32 Iterations);
	void PhysWallRun(float DeltaTime, int32 Iterations);
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	virtual void PhysWalking(float DeltaTime, int32 Iterations) override;
	class AHWBaseCharacter* GetBaseCharacterOwner() const;

	UPROPERTY(Category = "Character Movement: Swimming", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float SwimmingCapsuleRadius = 60.f;

	UPROPERTY(Category = "Character Movement: Swimming", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float SwimmingCapsuleHalfHeight = 60.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement: Sprint", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SprintSpeed = 1200.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement: Sprint", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float OutOfStaminaMaxSpeed = 100.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Prone", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float ProneCapsuleRadius = 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Prone", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float ProneCapsuleHalfHeight = 40.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Prone", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxProneSpeed = 100.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxLadderSpeed = 200.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float LadderBreakingDeceleration = 2048.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float LadderToCharacterOffset = 60.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxLadderHeightTopOffset = 90.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinLadderBottomOffset = 90.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float JumpOffFromLadderSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Zipline", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float ZiplineSpeed = 800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Zipline", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float JumpOffFromZiplineSpeed = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Wall run", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxWallRunTime = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Wall run", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxWallRunSpeed = 800.0f;

	UPROPERTY(Category = "Character Movement: Slide", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float SlideSpeed = 1000.0f;

	UPROPERTY(Category = "Character Movement: Slide", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float SlideCapsuleHalfHeight = 60.0f;

	UPROPERTY(Category = "Character Movement: Slide", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float SlideTime = 1.5f;

private:
	bool bIsSprinting = false;
	bool bIsOutOfStamina = false;
	bool bIsProne = false;
	bool bForceRotation = false;

	FMantlingMovementParameters CurrentMantlingParameters;
	FTimerHandle WallRunTimer;
	FTimerHandle CustomMovementTimer;
	const ALadder* CurrentLadder = nullptr;
	const AZipline* CurrentZipline = nullptr;
	FRotator ForceTargetRotation = FRotator::ZeroRotator;
	FVector CustomMoveDirection = FVector::ZeroVector;
	EWallRunSide WallRunSide = EWallRunSide::None;
};

// ReSharper disable once CppClassNeedsConstructorBecauseOfUninitializedMember
class FSavedMove_HW : public FSavedMove_Character
{
	typedef FSavedMove_Character Super;

public:

	virtual bool CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter, float MaxDelta) const override;

	virtual void Clear() override;

	virtual uint8 GetCompressedFlags() const override;

	virtual void PrepMoveFor(ACharacter* Character) override;

	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;

private:
	uint8 bSavedIsSprinting : 1;
	uint8 bSavedIsMantling : 1;
	uint8 bSavedIsSlideRequested : 1;
};

class FNetworkPredictionData_Client_Character_HW : public FNetworkPredictionData_Client_Character
{
    typedef FNetworkPredictionData_Client_Character Super;

public:
	FNetworkPredictionData_Client_Character_HW(const UHWBaseCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;

};
