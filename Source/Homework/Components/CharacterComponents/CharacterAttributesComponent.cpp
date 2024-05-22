// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAttributesComponent.h"

#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PhysicsVolume.h"
#include "Homework/HomeworkTypes.h"
#include "Homework/Characters/HWBaseCharacter.h"
#include "Homework/Subsystems/DebugSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCharacterAttributesComponent::UCharacterAttributesComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UCharacterAttributesComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCharacterAttributesComponent, Health);
	DOREPLIFETIME(UCharacterAttributesComponent, Stamina);
	DOREPLIFETIME(UCharacterAttributesComponent, Oxygen);
}

void UCharacterAttributesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateStamina(DeltaTime);
	UpdateOxygen(DeltaTime);
	
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	DebugDrawAttributes();
#endif
}

void UCharacterAttributesComponent::AddStamina(float StaminaToAdd)
{
	Stamina = FMath::Clamp(Stamina + StaminaToAdd, 0.f, MaxStamina);
	OnAttributeChanged(EAttributes::Stamina, Stamina / MaxStamina);
}

void UCharacterAttributesComponent::AddHealth(float HealthToAdd)
{
	Health = FMath::Clamp(Health + HealthToAdd, 0.f, MaxHealth);
	OnAttributeChanged(EAttributes::Health, Health / MaxHealth);
}

bool UCharacterAttributesComponent::IsAlive() const
{
	return Health > 0.f;
}

float UCharacterAttributesComponent::GetHealthPercent() const
{
	return Health / MaxHealth;
}

void UCharacterAttributesComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(MaxHealth > 0.f, TEXT("UCharacterAttributesComponent::BeginPlay MaxHealth must be positive value"));
	checkf(GetOwner()->IsA<AHWBaseCharacter>(), TEXT("UCharacterAttributesComponent::BeginPlay owner is not AHWBaseCharacter"));
	CachedBaseCharacterOwner = StaticCast<AHWBaseCharacter*>(GetOwner());
	
	Health = MaxHealth;
	Stamina = MaxStamina;
	Oxygen = MaxOxygen;

	if (GetOwner()->HasAuthority())
	{
		CachedBaseCharacterOwner->OnTakeAnyDamage.AddDynamic(this, &UCharacterAttributesComponent::OnTakeAnyDamage);
	}
}

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
void UCharacterAttributesComponent::DebugDrawAttributes() const
{
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	if (!DebugSubsystem->IsCategoryEnabled(DebugCategoryCharacterAttributes))
	{
		return;
	}

	FVector TextLocation = CachedBaseCharacterOwner->GetActorLocation() + FVector(0.f,0.f,CachedBaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	DrawDebugString(GetWorld(), TextLocation, FString::Printf(TEXT("Stamina: %.2f"), Stamina), nullptr, FColor::Green, 0, true);
	TextLocation.Z += 5.f;
	DrawDebugString(GetWorld(), TextLocation, FString::Printf(TEXT("Health: %.2f"), Health), nullptr, FColor::Red, 0, true);
	TextLocation.Z += 5.f;
	DrawDebugString(GetWorld(), TextLocation, FString::Printf(TEXT("Oxygen: %.2f"), Oxygen), nullptr, FColor::Cyan, 0, true);
}
#endif

void UCharacterAttributesComponent::OnAttributeChanged(EAttributes Attribute, float AttributePercent) const
{
	if (OnAttributeChangedDelegate.IsBound())
	{
		OnAttributeChangedDelegate.Broadcast(Attribute, AttributePercent);
	}
}

void UCharacterAttributesComponent::UpdateStamina(float DeltaSeconds)
{
	float DeltaStamina;
	if (CachedBaseCharacterOwner->IsSprinting())
	{
		DeltaStamina = -SprinStaminaConsumptionRate * DeltaSeconds;
	}
	else
	{
		DeltaStamina = StaminaRestoreRate * DeltaSeconds;
	}
	Stamina = FMath::Clamp(Stamina + DeltaStamina, 0.0f, MaxStamina);
	OnStaminaChanged();
}

void UCharacterAttributesComponent::UpdateOxygen(float DeltaSeconds)
{
	float DeltaOxygen;
	if (CachedBaseCharacterOwner->IsSwimmingUnderWater())
	{
		DeltaOxygen = -SwimOxygenConsumptionRate * DeltaSeconds;
	}
	else
	{
		DeltaOxygen = OxygenRestoreRate * DeltaSeconds;
		if (NoOxygenDamageTimer.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(NoOxygenDamageTimer);
		}
	}
	Oxygen = FMath::Clamp(Oxygen + DeltaOxygen, 0.0f, MaxOxygen);
	OnOxygenChanged();

}

void UCharacterAttributesComponent::TakeNoOxygenDamage()
{
	GetWorld()->GetTimerManager().ClearTimer(NoOxygenDamageTimer);
	AController* Instigator = CachedBaseCharacterOwner->GetController();
	APhysicsVolume* DamagingVolume = CachedBaseCharacterOwner->GetCharacterMovement()->GetPhysicsVolume();
	CachedBaseCharacterOwner->TakeDamage(NoOxygenDamage, FDamageEvent(), Instigator, DamagingVolume);
}

void UCharacterAttributesComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (!IsAlive())
	{
		return;
	}
	FString DamagedActorName = IsValid(DamagedActor) ? DamagedActor->GetName() : FString("NULL");
	FString DamageCauserName = IsValid(DamageCauser) ? DamageCauser->GetName() : FString("NULL");
	UE_LOG(LogDamage, Warning, TEXT("UCharacterAttributesComponent::OnTakeAnyDamage %s recieved %.2f damage from %s"), *DamagedActorName, Damage, *DamageCauserName);
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	OnHealthChanged();
}

void UCharacterAttributesComponent::OnRep_Health()
{
	OnHealthChanged();
}

void UCharacterAttributesComponent::OnHealthChanged()
{
	OnAttributeChanged(EAttributes::Health, Health / MaxHealth);
}

void UCharacterAttributesComponent::OnRep_Stamina()
{
	OnStaminaChanged();
}

void UCharacterAttributesComponent::OnStaminaChanged()
{
	OnAttributeChanged(EAttributes::Stamina, Stamina / MaxStamina);
}

void UCharacterAttributesComponent::OnRep_Oxygen()
{
	OnOxygenChanged();
}

void UCharacterAttributesComponent::OnOxygenChanged()
{
	OnAttributeChanged(EAttributes::Oxygen, Oxygen / MaxOxygen);
	if (Oxygen <= 0.f && !NoOxygenDamageTimer.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(NoOxygenDamageTimer, this,
		                                       &UCharacterAttributesComponent::TakeNoOxygenDamage,
		                                       NoOxygenDamageCooldown);
	}
}
