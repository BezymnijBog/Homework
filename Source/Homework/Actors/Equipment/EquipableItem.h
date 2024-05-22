// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "HomeworkTypes.h"
#include "GameFramework/Actor.h"
#include "EquipableItem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentStateChanged, bool, bIsEquipped);

class AHWBaseCharacter;

UCLASS(Abstract, NotBlueprintable)
class HOMEWORK_API AEquipableItem : public AActor
{
	GENERATED_BODY()

public:
	AEquipableItem();
	
	virtual void Equip();
	virtual void UnEquip();
	virtual void SetOwner(AActor* NewOwner) override;

	bool IsSlotCompatible(EEquipmentSlots Slot) const;
	
	AHWBaseCharacter* GetBaseCharacterOwner() const;
	EEquipableItemType GetItemType() const;
	virtual EReticleType GetReticleType() const;
	FName GetDataTableID() const;
	FName GetEquippedSocketName() const;
	FName GetUnEquippedSocketName() const;
	UAnimMontage* GetCharacterEquipAnimMontage() const;
	
protected:
	UPROPERTY(BlueprintAssignable)
	FOnEquipmentStateChanged OnEquipmentStateChanged;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipable Item")
	EEquipableItemType ItemType = EEquipableItemType::None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipable Item")
	FName EquippedSocketName = NAME_None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipable Item")
	FName UnEquippedSocketName = NAME_None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipable Item")
	UAnimMontage* CharacterEquipAnimMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reticle")
	EReticleType ReticleType = EReticleType::None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName DataTableID = NAME_None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TArray<EEquipmentSlots> CompatibleSlots;

private:
	TWeakObjectPtr<AHWBaseCharacter> CachedBaseCharacterOwner;
};
