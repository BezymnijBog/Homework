// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PawnAttributesComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnDeathDelegate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOMEWORK_API UPawnAttributesComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPawnAttributesComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	bool IsAlive() const;
	
	FOnDeathDelegate OnDeathDelegate;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes | Health")
	float MaxHealth = 100.f;

private:
	UPROPERTY(ReplicatedUsing = OnRep_Health, SaveGame)
	float Health = 0.f;
	UFUNCTION()
	void OnRep_Health();
	void OnHealthChanged();

	UFUNCTION()
	void OnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
};
