// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "HomeworkTypes.h"
#include "Components/ActorComponent.h"
#include "CharacterEquipmentComponent.generated.h"

class UWeaponWheelWidget;
class UEquipmentViewWidget;
class AThrowableItem;
class AEquipableItem;
class AHWBaseCharacter;
class AMeleeWeaponItem;
class ARangeWeaponItem;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquippedItemChanged, const AEquipableItem*)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCurrentWeaponAmmoChanged, int32, uint32)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGrenadeCountChanged, int32, int32)

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOMEWORK_API UCharacterEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UCharacterEquipmentComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void AddAmmo(EAmunitionType AmmoType, int32 AmmoCount);
	bool AddEquipmentItemToSlot(const TSubclassOf<AEquipableItem> ItemClass, int32 SlotIndex);
	void AttachCurrentItemToEquippedSocket();
	void AutoEquip();
	void CloseViewEquipment();
	void ConfirmWeaponSelection();
	void EquipItemInSlot(EEquipmentSlots Slot);
	void EquipNextItem();
	void EquipPreviousItem();
	void LaunchCurrentThrowableItem();
	void OpenViewEquipment(APlayerController* PlayerController);
	void OpenWeaponWheel(APlayerController* PlayerController);
	void ReloadAmmoInCurrentWeapon(int32 NumberOfAmmo = 0, bool bCheckIsFull = false);
	void ReloadCurrentWeapon();
	void RemoveAmmo(EAmunitionType AmunitionType);
	void RemoveItemFromSlot(int32 SlotIndex);
	void UnequipCurrentItem();

	bool IsEquipping() const;
	bool IsViewVisible() const;
	bool IsSelectingWeapon() const;
	
	EEquipableItemType GetCurrentEquippedItemType() const;
	AMeleeWeaponItem* GetCurrentMeleeWeapon() const;
	ARangeWeaponItem* GetCurrentRangeWeapon() const;
	int32 GetGrenadesAmount() const;
	const TArray<AEquipableItem*>& GetItems() const;
	uint32 NextItemsArraySlotIndex(uint32 CurrentSlotIndex) const;
	uint32 PreviousItemsArraySlotIndex(uint32 CurrentSlotIndex) const;

	FOnCurrentWeaponAmmoChanged OnCurrentWeaponAmmoChangedEvent;
	FOnEquippedItemChanged OnEquippedItemChanged;
	FOnGrenadeCountChanged OnGrenadeCountChangedEvent;

protected:
	virtual void BeginPlay() override;

	void CreateEquipmentWidgets(APlayerController* PlayerController);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	TMap<EAmunitionType, int32> MaxAmunitionAmount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	TMap<EEquipmentSlots, TSubclassOf<AEquipableItem>> ItemsLoadout;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	TSet<EEquipmentSlots> IgnoreSlotsWhileSwitching;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	EEquipmentSlots AutoEquipItemSlot = EEquipmentSlots::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "View")
	TSubclassOf<UEquipmentViewWidget> ViewWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "View")
	TSubclassOf<UWeaponWheelWidget> WeaponWheelWidgetClass;

private:
	void AddAmmoInternal(EAmunitionType AmmoType, int32 AmmoCount);
	bool CanThrowGrenade() const;
	void CreateLoadoutThrowables();
	void CreateLoadout();
	int32 GetAvailableAmunitionForCurrentWeapon() const;
	int32 GetGrenadesTotalAmount() const;

	UFUNCTION(Server, Reliable)
	void Server_AddAmmo(EAmunitionType AmmoType, int32 AmmoCount);
	
	UFUNCTION(Server, Reliable)
	void Server_EquipItemInSlot(EEquipmentSlots Slot);

	UFUNCTION()
	void OnWeaponReloadComplete();

	UFUNCTION()
	void OnWeaponUnloaded();
	
	UFUNCTION()
	void OnCurrentWeaponAmmoChanged(int32 Ammo) const;
	void OnGrenadeCountChanged() const;
	void OnEquipAnimationFinished();

	AEquipableItem* CurrentItem;
	ARangeWeaponItem* CurrentRangeWeapon;
	AThrowableItem* CurrentThrowableItem;
	AMeleeWeaponItem* CurrentMeleeItem;

	UPROPERTY()
	UEquipmentViewWidget* ViewWidget;
	UPROPERTY()
	UWeaponWheelWidget* WeaponWheelWidget;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentEquippedSlot)
	EEquipmentSlots CurrentEquippedSlot;
	EEquipmentSlots PreviousEquipSlot;

	UFUNCTION()
	void OnRep_CurrentEquippedSlot(EEquipmentSlots CurrentEquippedSlot_Old);
	
	FDelegateHandle OnCurrentWeaponAmmoChangedHandle;
	FDelegateHandle OnCurrentWeaponReloadedHandle;
	
	FTimerHandle EquipTimer;
	
	TWeakObjectPtr<AHWBaseCharacter> CachedBaseCharacter;

	UPROPERTY(ReplicatedUsing=OnRep_AmunitionArray)
	TArray<int32> AmunitionArray;

	UPROPERTY(ReplicatedUsing=OnRep_ItemsArray)
	TArray<AEquipableItem*> ItemsArray;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentGrenades)
	int32 CurrentGrenadesAmount = 0;

	UFUNCTION()
	void OnRep_AmunitionArray();
	
	UFUNCTION()
	void OnRep_ItemsArray();
	
	UFUNCTION()
	void OnRep_CurrentGrenades();
	
	bool bIsEquipping = false;
};
