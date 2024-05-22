// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"

#include "HomeworkTypes.h"
#include "GameFramework/Character.h"
#include "Components/MovementComponents/HWBaseCharacterMovementComponent.h"
#include "Subsystems/SaveSubsystem/SaveSubsystemInterface.h"


#include "HWBaseCharacter.generated.h"

class UCharacterInventoryComponent;
class UInventoryItem;
class AEquipableItem;
class UWidgetComponent;
class IInteractive;
class UCharacterAttributesComponent;
class UCharacterEquipmentComponent;
class AZipline;
class ALadder;
class AInteractiveActor;
class UHWBaseCharacterMovementComponent;

USTRUCT(BlueprintType)
struct FMantlingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* MantlingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* FPMantlingMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UCurveVector* MantlingCurve;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxHeight = 200.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float MinHeight = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxHeightStartTime = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float MinHeightStartTime = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float AnimationCorrectionXY = 65.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float AnimationCorrectionZ = 200.f;	
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimingStateChanged, bool)
DECLARE_DELEGATE_OneParam(FOnInteractiveObjectFound, FName)

UCLASS(Abstract, NotBlueprintable)
class HOMEWORK_API AHWBaseCharacter : public ACharacter, public IGenericTeamAgentInterface, public ISaveSubsystemInterface
{
	GENERATED_BODY()

public:
	AHWBaseCharacter(const FObjectInitializer& InObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;

	virtual void MoveForward(float InValue);
	virtual void MoveRight(float InValue);
	virtual void Turn(float InValue) {}
	virtual void LookUp(float InValue) {}
	virtual void TurnAtRate(float InValue) {}
	virtual void LookUpAtRate(float InValue) {}
	virtual void SwimForward(float InValue) {}
	virtual void SwimRight(float InValue) {}
	virtual void SwimUp(float InValue) {}
	void ClimbLadderUp(float InValue);
	void InteractWithLadder();
	void ZiplineInteraction();
	virtual void Jump() override;

	UFUNCTION(BlueprintCallable)
	virtual void Mantle(bool bForse = false);
	virtual void StartSlide();
	virtual void StopSlide();
	virtual void ChangeCrouchState();
	virtual void ChangeProneState();
	virtual void StartSprint();
	virtual void StopSprint();
	bool IsSprinting() const;
	void StartFire();
	void StopFire();
	void StartAim();
	void StopAim();
	float GetAimingMoveSpeed() const;
	bool IsAiming() const;
	void Reload();
	void NexItem();
	void PreviousItem();
	void EquipPrimaryItem();
	void PrimaryMeleeAttack();
	void SecondaryMeleeAttack();
	void Interact();
	void UseInventory(APlayerController* PlayerController);
	void SwitchWeaponMode();
	void DestroyActorOnServer(AActor* Actor) const;
	
	bool CanFire() const;
	bool PickupItem(UInventoryItem* Item);
	void ConfirmWeaponSelection();

	// @ ISaveSubsystemInterface
	virtual void OnLevelDeserialized_Implementation() override;
	// ~@ ISaveSubsystemInterface
	
	FOnAimingStateChanged OnAimingStateChanged;
	FOnInteractiveObjectFound OnInteractiveObjectFound;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character")
	void OnStartAim();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character")
	void OnStoptAim();

	virtual void Falling() override;
	virtual void NotifyJumpApex() override;
	virtual void Landed(const FHitResult& Hit) override;
	
	void RegisterInteractiveActor(AInteractiveActor* InteractiveActor);
	void UnRegisterInteractiveActor(AInteractiveActor* InteractiveActor);

	bool IsSwimmingUnderWater() const;
	bool AreRequiredKeysDown(EWallRunSide Side) const;
	void GetWallRunSideAndDirection(const FVector& HitNormal, EWallRunSide& OutSide, FVector& OutDirection) const;

	virtual void OnStartSliding(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnStartSliding", ScriptName = "OnStartSliding"))
	void K2_OnStartSliding(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

	virtual void OnEndSliding(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEndSliding", ScriptName = "OnEndSliding"))
	void K2_OnEndSliding(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	
	virtual void OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEndProne", ScriptName = "OnEndProne"))
	void K2_OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	
	virtual void OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnStartProne", ScriptName = "OnStartProne"))
	void K2_OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = Character, meta = (HidePin = "bClientSimulation"))
	virtual void LieDown(bool bClientSimulation = false);

	UFUNCTION(BlueprintCallable, Category = Character, meta = (HidePin = "bClientSimulation"))
	virtual void StandUp(bool bClientSimulation = false);

	UFUNCTION(BlueprintCallable, Category = Character)
	virtual bool CanBeProne() const;
	
	UFUNCTION(BlueprintCallable, Category = Character)
	bool IsProne() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetIKRightFootOffset() const { return IKRightFootOffset; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetIKLeftFootOffset() const { return IKLeftFootOffset; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetIKBodyOffset() const { return IKBodyOffset; }

	FRotator GetAimOffset();
	
	UHWBaseCharacterMovementComponent* GetBaseCharacterMovement() const;
	const UCharacterEquipmentComponent* GetEquipmentComponent() const;
	UCharacterEquipmentComponent* GetEquipmentComponent_Mutable() const;
	const UCharacterAttributesComponent* GetCharacterAttributes() const;
	UCharacterAttributesComponent* GetCharacterAttributes_Mutable();
	UCharacterInventoryComponent* GetCharacterInventory() const;

/**IGenericTeamAgentInterface*/
	virtual FGenericTeamId GetGenericTeamId() const override;
/**~IGenericTeamAgentInterface*/

	UPROPERTY(ReplicatedUsing=OnRep_IsMantling)
	bool bIsMantling = false;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_IsSliding)
	bool bIsSliding;
	bool bIsSlideRequested = false;

	UFUNCTION()
	void OnRep_IsMantling(bool bWasMantling);
	
	UFUNCTION()
	virtual void OnRep_IsSliding();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Controls")
	float BaseTurnRate = 45.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Controls")
	float BaseLookUpRate = 45.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Movement")
	float SprintSpeed = 800.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Hard landing", meta = (ClampMin = 0.f, UIMin = 0.f))
	float HardLandingHeight = 250.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Hard landing")
	UAnimMontage* HardLandingMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|IK settings", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float IKTraceExtendDistance = 30.0f;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Character|IK settings", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float IKInterpSpeed = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|IK settings")
	FName RightFootSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|IK settings")
	FName LeftFootSocketName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	UCharacterAttributesComponent* CharacterAttributesComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Movement")
	class ULedgeDetectorComponent* LedgeDetectorComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling")
	FMantlingSettings HighMantleSettings;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling")
	FMantlingSettings LowMantleSettings;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling", meta = (ClampMin = 0.f, UIMin = 0.f))
	float LowMantleMaxHeight = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Animations")
	UAnimMontage* OnDeathAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Animations")
	UAnimMontage* SlidingAnimMontage;

	// damage depending on fall height (in meters)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charater | Attributes")
	UCurveFloat* FallDamageCurve;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	class UCharacterEquipmentComponent* CharacterEquipmentComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	UHWBaseCharacterMovementComponent* BaseCharacterMovementComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	UCharacterInventoryComponent* CharacterInventoryComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Team")
	ETeams Team = ETeams::Enemy;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Sight")
	float LineOfSightDistance = 500.f;

	UPROPERTY()
	TScriptInterface<IInteractive> LineOfSightObject;

	UPROPERTY(VisibleDefaultsOnly, Category = "Character | Components")
	UWidgetComponent* HealthBarProgressComponent;

	/**********************************************************************/	
	UFUNCTION(BlueprintNativeEvent, Category = "Character | Movement")
	void OnSprintStart();
	virtual void OnSprintStart_Implementation();
	UFUNCTION(BlueprintNativeEvent, Category = "Character | Movement")
	void OnSprintStop();
	virtual void OnSprintStop_Implementation();
	virtual bool CanSprint();
	virtual bool CanJumpInternal_Implementation() const override;
	virtual bool CanMantle() const;
	virtual bool CanInteractWithLadder() const;
	virtual bool CanCrouch() const override;
	virtual void OnStartAimInternal();
	virtual void OnStopAimInternal();
	virtual void OnDeath();
	void OnOutOfStamina(bool bIsOutOfStamina);
	void OnHealthChanged(float AttributePercent);
	void OnAttributeChanged(EAttributes Attribute, float AttributePercent);

	void InitializeHealthProgress();

	UFUNCTION()
	void OnPlayerCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime);

	void TraceLineOfSight();
	
	FVector CurrentFallApex = FVector::ZeroVector;
	
private:
	UFUNCTION(Server, Reliable)
	void Server_DestroyActor(AActor* Actor) const;
	
	template <typename T>
	const T* GetAvailable() const;

	bool IsSurfaceWallRunable(const FVector& Normal) const;
	void TryChangeSprintState();
	void TryChangeSlideState();
	float GetIKOffsetForASocket(const FName& SocketName);
	float GetIKOffsetForBody() const;
	const FMantlingSettings& GetMantlingSettings(float LedgeHeight) const;
	void EnableRagdoll();
	
	UPROPERTY(ReplicatedUsing=OnRep_Aiming)
	bool bIsAiming = false;

	UFUNCTION()
	void OnRep_Aiming(bool bIsAiming_Old);

	UFUNCTION(Server, Reliable)
	void Server_StartAim();
	void Server_StartAim_Implementation();
	
	void StartAimInternal();
	
	UFUNCTION(Server, Reliable)
	void Server_StopAim();
	void Server_StopAim_Implementation();
	
	void StopAimInternal();
	
	float IKRightFootOffset = 0.0f;
	float IKLeftFootOffset = 0.0f;
	float IKBodyOffset = 0.0f;
	
	float ForwardAxis = 0.0f;
	float RightAxis = 0.0f;
	float CurrentAimingMovementSpeed = 0.f;
	
	bool bIsSprintRequested = false;

	FTimerHandle Timer;
	TArray<AInteractiveActor*> AvailableInteractiveActors;
};
