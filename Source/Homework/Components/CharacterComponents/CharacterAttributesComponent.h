// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HomeworkTypes.h"
#include "CharacterAttributesComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAttributeChanged, EAttributes, float);

class AHWBaseCharacter;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOMEWORK_API UCharacterAttributesComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCharacterAttributesComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void AddStamina(float StaminaToAdd);
	void AddHealth(float HealthToAdd);
	
	bool IsAlive() const;
	float GetHealthPercent() const;

	FOnAttributeChanged OnAttributeChangedDelegate;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (UIMin = 0.0f))
	float MaxHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (UIMin = 0.0f))
	float MaxStamina = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (UIMin = 0.0f))
	float StaminaRestoreRate = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (UIMin = 0.0f))
	float SprinStaminaConsumptionRate = 12.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.0f))
	float MaxOxygen = 50.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.0f))
	float OxygenRestoreRate = 15.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.0f))
	float SwimOxygenConsumptionRate = 2.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.0f))
	float NoOxygenDamage = 5.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.0f))
	float NoOxygenDamageCooldown = 2.0f;
	
private:
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	void DebugDrawAttributes() const;
#endif

	void OnAttributeChanged(EAttributes Attribute, float AttributePercent) const;
	
	void UpdateStamina(float DeltaSeconds);
	void UpdateOxygen(float DeltaSeconds);
	void TakeNoOxygenDamage();
	
	UFUNCTION()
	void OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(ReplicatedUsing=OnRep_Health)
	float Health = 0.f;
	UFUNCTION()
	void OnRep_Health();
	void OnHealthChanged();
	
	UPROPERTY(ReplicatedUsing=OnRep_Health)
	float Stamina = 0.f;
	UFUNCTION()
	void OnRep_Stamina();
	void OnStaminaChanged();
	
	UPROPERTY(ReplicatedUsing=OnRep_Health)
	float Oxygen = 0.f;
	UFUNCTION()
	void OnRep_Oxygen();
	void OnOxygenChanged();

	
	FTimerHandle NoOxygenDamageTimer;
	TWeakObjectPtr<AHWBaseCharacter> CachedBaseCharacterOwner;
};
