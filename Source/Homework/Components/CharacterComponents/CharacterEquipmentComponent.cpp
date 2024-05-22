// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterEquipmentComponent.h"


#include "CharacterInventoryComponent.h"
#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Characters/HWBaseCharacter.h"
#include "HomeworkTypes.h"
#include "Actors/Equipment/Trowables/ThrowableItem.h"
#include "Inventory/Items/Equipables/AmmoInventoryItem.h"
#include "Net/UnrealNetwork.h"
#include "UI/Widget/Equipment/EquipmentViewWidget.h"
#include "UI/Widget/Equipment/WeaponWheelWidget.h"
#include "Utils/HWDataTableUtils.h"

UCharacterEquipmentComponent::UCharacterEquipmentComponent()
{
	SetIsReplicatedByDefault(true);
}

void UCharacterEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCharacterEquipmentComponent, CurrentEquippedSlot);
	DOREPLIFETIME(UCharacterEquipmentComponent, AmunitionArray);
	DOREPLIFETIME(UCharacterEquipmentComponent, ItemsArray);
	DOREPLIFETIME(UCharacterEquipmentComponent, CurrentGrenadesAmount);
}

void UCharacterEquipmentComponent::AddAmmo(EAmunitionType AmmoType, int32 AmmoCount)
{
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		AddAmmoInternal(AmmoType, AmmoCount);
	}
	Server_AddAmmo(AmmoType, AmmoCount);
}

bool UCharacterEquipmentComponent::AddEquipmentItemToSlot(const TSubclassOf<AEquipableItem> ItemClass, int32 SlotIndex)
{
	AEquipableItem* DefaultItemObject = ItemClass.GetDefaultObject();
	EEquipmentSlots Slot = StaticCast<EEquipmentSlots>(SlotIndex);
	if (!DefaultItemObject->IsSlotCompatible(Slot))
	{
		return false;
	}
	
	if (!IsValid(ItemsArray[SlotIndex]))
	{
		AEquipableItem* Item = GetWorld()->SpawnActor<AEquipableItem>(ItemClass);
		Item->AttachToComponent(CachedBaseCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, Item->GetUnEquippedSocketName());
		Item->SetOwner(CachedBaseCharacter.Get());
		Item->UnEquip();
		ItemsArray[SlotIndex] = Item;
	}
	else if (DefaultItemObject->IsA<ARangeWeaponItem>())
	{
		ARangeWeaponItem* RangeWeapon = StaticCast<ARangeWeaponItem*>(DefaultItemObject);
		int32 AmmoSlotIndex = StaticCast<int32>(RangeWeapon->GetAmmoType());
		AmunitionArray[AmmoSlotIndex] = RangeWeapon->GetMaxAmmo();
	}
	
	return true;
}

void UCharacterEquipmentComponent::AttachCurrentItemToEquippedSocket()
{
	if (IsValid(CurrentItem)) 
	{
		CurrentItem->AttachToComponent(CachedBaseCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, CurrentItem->GetEquippedSocketName());
	}
}

void UCharacterEquipmentComponent::AutoEquip()
{
	if (AutoEquipItemSlot != EEquipmentSlots::None)
	{
		EquipItemInSlot(AutoEquipItemSlot);
	}
}

void UCharacterEquipmentComponent::CloseViewEquipment()
{
	if (IsValid(ViewWidget) && ViewWidget->IsVisible())
	{
		ViewWidget->RemoveFromParent();
	}
}

void UCharacterEquipmentComponent::ConfirmWeaponSelection()
{
	WeaponWheelWidget->ConfirmSelection();
}

void UCharacterEquipmentComponent::EquipItemInSlot(EEquipmentSlots Slot)
{
	int32 ItemsIdx = StaticCast<uint32>(Slot);
	if (bIsEquipping || ItemsArray.Num() <= ItemsIdx)
	{
		return;
	}
	if (Slot == EEquipmentSlots::PrimaryItemSlot && !CanThrowGrenade())
	{
		return;
	}
	
	UnequipCurrentItem();
	CurrentItem = ItemsArray[ItemsIdx];
	CurrentRangeWeapon = Cast<ARangeWeaponItem>(CurrentItem);
	CurrentThrowableItem = Cast<AThrowableItem>(CurrentItem);
	CurrentMeleeItem = Cast<AMeleeWeaponItem>(CurrentItem);
	
	if (IsValid(CurrentItem))
	{
		UAnimMontage* EquipMontage = CurrentItem->GetCharacterEquipAnimMontage();
		if (IsValid(EquipMontage) && !CachedBaseCharacter->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
		{
			bIsEquipping = true;
			float Duration = CachedBaseCharacter->PlayAnimMontage(EquipMontage);
			GetWorld()->GetTimerManager().SetTimer(EquipTimer, this, &UCharacterEquipmentComponent::OnEquipAnimationFinished, Duration);
		}
		else
		{
			AttachCurrentItemToEquippedSocket();
		}
		CurrentEquippedSlot = Slot;
		CurrentItem->Equip();
		if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_EquipItemInSlot(CurrentEquippedSlot);
		}
	}
	if (IsValid(CurrentRangeWeapon))
	{
		OnCurrentWeaponAmmoChangedHandle = CurrentRangeWeapon->OnAmmoChanged.AddUFunction(this, FName("OnCurrentWeaponAmmoChanged"));
		OnCurrentWeaponReloadedHandle = CurrentRangeWeapon->OnReloadComplete.AddUFunction(this, FName("OnWeaponReloadComplete"));
		OnCurrentWeaponReloadedHandle = CurrentRangeWeapon->OnUnloaded.AddUFunction(this, FName("OnWeaponUnloaded"));
		OnCurrentWeaponAmmoChanged(CurrentRangeWeapon->GetAmmo());
	}

	if (OnEquippedItemChanged.IsBound())
	{
		OnEquippedItemChanged.Broadcast(CurrentItem);
	}
}

void UCharacterEquipmentComponent::EquipNextItem()
{
	if (CachedBaseCharacter->IsPlayerControlled())
	{
		if (IsSelectingWeapon())
		{
			WeaponWheelWidget->NextSegment();
		}
		else
		{
			APlayerController* PlayerController = CachedBaseCharacter->GetController<APlayerController>();
			OpenWeaponWheel(PlayerController);
		}
		return;
	}
	
	uint32 CurrentSlotIndex = StaticCast<uint32>(CurrentEquippedSlot);
	uint32 NextSlotIndex = NextItemsArraySlotIndex(CurrentSlotIndex);
	EEquipmentSlots Slot = StaticCast<EEquipmentSlots>(NextSlotIndex);
	while (CurrentSlotIndex != NextSlotIndex && !IsValid(ItemsArray[NextSlotIndex]) || IgnoreSlotsWhileSwitching.Contains(Slot))
	{
		NextSlotIndex = NextItemsArraySlotIndex(NextSlotIndex);
		Slot = StaticCast<EEquipmentSlots>(NextSlotIndex);
	}
	if (CurrentSlotIndex != NextSlotIndex)
	{
		EquipItemInSlot(Slot);
	}
}

void UCharacterEquipmentComponent::EquipPreviousItem()
{
	if (CachedBaseCharacter->IsPlayerControlled())
	{
		if (IsSelectingWeapon())
		{
			WeaponWheelWidget->PreviousSegment();
		}
		else
		{
			APlayerController* PlayerController = CachedBaseCharacter->GetController<APlayerController>();
			OpenWeaponWheel(PlayerController);
		}
		return;
	}
	
	uint32 CurrentSlotIndex = StaticCast<uint32>(CurrentEquippedSlot);
	uint32 PreviousSlotIndex = PreviousItemsArraySlotIndex(CurrentSlotIndex);
	EEquipmentSlots Slot = StaticCast<EEquipmentSlots>(PreviousSlotIndex);
	while (CurrentSlotIndex != PreviousSlotIndex && !IsValid(ItemsArray[PreviousSlotIndex]) || IgnoreSlotsWhileSwitching.Contains(Slot))
	{
		PreviousSlotIndex = PreviousItemsArraySlotIndex(PreviousSlotIndex);
		Slot = StaticCast<EEquipmentSlots>(PreviousSlotIndex);
	}
	if (CurrentSlotIndex != PreviousSlotIndex)
	{
		EquipItemInSlot(Slot);
	}
}

void UCharacterEquipmentComponent::LaunchCurrentThrowableItem()
{
	if (IsValid(CurrentThrowableItem) && GetOwnerRole() == ROLE_Authority)
	{
		CurrentThrowableItem->Throw();
		bIsEquipping = false;
		EquipItemInSlot(PreviousEquipSlot);
		CurrentGrenadesAmount--;
		OnGrenadeCountChanged();
	}
}

void UCharacterEquipmentComponent::OpenViewEquipment(APlayerController* PlayerController)
{
	if (!IsValid(ViewWidget))
	{
		CreateEquipmentWidgets(PlayerController);
	}
	
	if (!ViewWidget->IsVisible())
	{
		ViewWidget->AddToViewport();
	}
}

void UCharacterEquipmentComponent::OpenWeaponWheel(APlayerController* PlayerController)
{
	if (!IsValid(WeaponWheelWidget))
	{
		CreateEquipmentWidgets(PlayerController);
	}

	if (!WeaponWheelWidget->IsVisible())
	{
		WeaponWheelWidget->AddToViewport();
	}
}

void UCharacterEquipmentComponent::ReloadAmmoInCurrentWeapon(int32 NumberOfAmmo, bool bCheckIsFull)
{
	int32 AvailableAmunition = GetAvailableAmunitionForCurrentWeapon();	
	int32 CurrentAmmo = CurrentRangeWeapon->GetAmmo();
	int32 AmmoToReload = CurrentRangeWeapon->GetMaxAmmo() - CurrentAmmo;
	int32 ReloadedAmmo = FMath::Min( AvailableAmunition, AmmoToReload);
	if (NumberOfAmmo > 0)
	{
		ReloadedAmmo = FMath::Min(ReloadedAmmo, NumberOfAmmo);
	}
	
	uint32 AmmoType = StaticCast<uint32>(CurrentRangeWeapon->GetAmmoType());
	if (GetOwnerRole() == ROLE_Authority)
	{
		AmunitionArray[AmmoType] -= ReloadedAmmo;
	}
	CurrentRangeWeapon->SetAmmo(ReloadedAmmo + CurrentAmmo);
	bool bIsFullyReloaded = CurrentRangeWeapon->GetAmmo() == CurrentRangeWeapon->GetMaxAmmo();	
	if (bCheckIsFull && (AmunitionArray[AmmoType] == 0 || bIsFullyReloaded))
	{
		CurrentRangeWeapon->EndReload(true);
	}
}

void UCharacterEquipmentComponent::ReloadCurrentWeapon()
{
	check(IsValid(CurrentRangeWeapon));
	if (GetAvailableAmunitionForCurrentWeapon() <= 0)
	{
		return;
	}
	CurrentRangeWeapon->StartReload();
}

void UCharacterEquipmentComponent::RemoveAmmo(EAmunitionType AmunitionType)
{
	int32 AmmoIndex = StaticCast<int32>(AmunitionType);
	if (AmmoIndex < AmunitionArray.Num())
	{
		AmunitionArray[AmmoIndex] = 0;
	}
	for (AEquipableItem* Item : ItemsArray)
	{
		if (ARangeWeaponItem* RangeWeapon = Cast<ARangeWeaponItem>(Item))
		{
			RangeWeapon->RemoveAmmo(AmunitionType);
			if (RangeWeapon == CurrentRangeWeapon)
			{
				OnCurrentWeaponAmmoChanged(0);
			}
		}
	}
}

void UCharacterEquipmentComponent::RemoveItemFromSlot(int32 SlotIndex)
{
	uint32 CurrentSlotIndex = StaticCast<uint32>(CurrentEquippedSlot);
	if (CurrentSlotIndex == SlotIndex)
	{
		UnequipCurrentItem();
	}
	ItemsArray[SlotIndex]->Destroy();
	ItemsArray[SlotIndex] = nullptr;
}

void UCharacterEquipmentComponent::UnequipCurrentItem()
{
	if (IsValid(CurrentItem))
	{
		CurrentItem->AttachToComponent(CachedBaseCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, CurrentItem->GetUnEquippedSocketName());
		CurrentItem->UnEquip();
	}
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StopFire();
		CurrentRangeWeapon->EndReload(false);
		CurrentRangeWeapon->OnAmmoChanged.Remove(OnCurrentWeaponAmmoChangedHandle);
		CurrentRangeWeapon->OnReloadComplete.Remove(OnCurrentWeaponReloadedHandle);
	}
	PreviousEquipSlot = CurrentEquippedSlot;
	CurrentEquippedSlot = EEquipmentSlots::None;
}

bool UCharacterEquipmentComponent::IsEquipping() const
{
	return bIsEquipping;
}

bool UCharacterEquipmentComponent::IsViewVisible() const
{
	return IsValid(ViewWidget) && ViewWidget->IsVisible();
}

bool UCharacterEquipmentComponent::IsSelectingWeapon() const
{
	return IsValid(WeaponWheelWidget) && WeaponWheelWidget->IsVisible();
}

EEquipableItemType UCharacterEquipmentComponent::GetCurrentEquippedItemType() const
{
	EEquipableItemType Result = EEquipableItemType::None;
	if (IsValid(CurrentItem))
	{
		Result = CurrentItem->GetItemType();
	}
	return Result;
}

AMeleeWeaponItem* UCharacterEquipmentComponent::GetCurrentMeleeWeapon() const
{
	return CurrentMeleeItem;
}

ARangeWeaponItem* UCharacterEquipmentComponent::GetCurrentRangeWeapon() const
{
	return CurrentRangeWeapon;
}

int32 UCharacterEquipmentComponent::GetGrenadesAmount() const
{
	return CurrentGrenadesAmount;
}

const TArray<AEquipableItem*>& UCharacterEquipmentComponent::GetItems() const
{
	return ItemsArray;
}

uint32 UCharacterEquipmentComponent::NextItemsArraySlotIndex(uint32 CurrentSlotIndex) const
{
	if (CurrentSlotIndex == ItemsArray.Num() - 1)
	{
		return 0;
	}
	return ++CurrentSlotIndex;
}

uint32 UCharacterEquipmentComponent::PreviousItemsArraySlotIndex(uint32 CurrentSlotIndex) const
{
	if (CurrentSlotIndex == 0)
	{
		return ItemsArray.Num() - 1;
	}
	return --CurrentSlotIndex;
}

void UCharacterEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	checkf(GetOwner()->IsA<AHWBaseCharacter>(), TEXT("UCharacterEquipmentComponent::BeginPlay(): UCharacterEquipmentComponent can be used only with AHWBaseCharacter"));
	CachedBaseCharacter = StaticCast<AHWBaseCharacter*>(GetOwner());
	CreateLoadout();
	AutoEquip();
}

void UCharacterEquipmentComponent::CreateEquipmentWidgets(APlayerController* PlayerController)
{
	check(IsValid(ViewWidgetClass));
	check(IsValid(WeaponWheelWidgetClass));

	if (IsValid(PlayerController))
	{
		ViewWidget = CreateWidget<UEquipmentViewWidget>(PlayerController, ViewWidgetClass);
		ViewWidget->InitializeEquipmentWidget(this);

		WeaponWheelWidget = CreateWidget<UWeaponWheelWidget>(PlayerController, WeaponWheelWidgetClass);
		WeaponWheelWidget->InitializeWeaponWheelWidget(this);
	}
}

void UCharacterEquipmentComponent::AddAmmoInternal(EAmunitionType AmmoType, int32 AmmoCount)
{
	int32 AmmoIndex = StaticCast<int32>(AmmoType);
	while (AmmoIndex >= AmunitionArray.Num())
	{
		AmunitionArray.Add(0);
	}
	AmunitionArray[AmmoIndex] += AmmoCount;
	if (IsValid(CurrentRangeWeapon))
	{
		OnCurrentWeaponAmmoChanged(CurrentRangeWeapon->GetAmmo());
	}
}

bool UCharacterEquipmentComponent::CanThrowGrenade() const
{
	return CurrentGrenadesAmount > 0;
}

void UCharacterEquipmentComponent::CreateLoadoutThrowables()
{
	if (MaxAmunitionAmount.Contains(EAmunitionType::FragGrenade))
	{
		CurrentGrenadesAmount = MaxAmunitionAmount[EAmunitionType::FragGrenade];
		OnGrenadeCountChanged();
	}
}

void UCharacterEquipmentComponent::CreateLoadout()
{
	if (GetOwner()->GetLocalRole() < ROLE_Authority)
	{
		return;
	}
	
	AmunitionArray.AddZeroed(StaticCast<uint32>(EAmunitionType::MAX));
	for (const TPair<EAmunitionType, int32>& AmmoPair : MaxAmunitionAmount)
	{
		uint32 ArrayKey = StaticCast<uint32>(AmmoPair.Key);
		AmunitionArray[ArrayKey] = FMath::Max(AmmoPair.Value, 0);
		FName AmmoID;
		if (FAmmoTableRow* AmmoRow = HWDataTableUtils::FindAmmoData(AmmoPair.Key, AmmoID))
		{
			UAmmoInventoryItem* AmmoItem = NewObject<UAmmoInventoryItem>();
			AmmoItem->Initialize(AmmoID, AmmoRow->AmmoItemDescription);
			AmmoItem->SetAmmoType(AmmoPair.Key);
			CachedBaseCharacter->GetCharacterInventory()->AddItem(AmmoItem, AmunitionArray[ArrayKey]);
		}
	}
	CreateLoadoutThrowables();
	ItemsArray.AddZeroed(StaticCast<uint32>(EEquipmentSlots::MAX));
	for (const TPair<EEquipmentSlots, TSubclassOf<AEquipableItem>>& ItemPair : ItemsLoadout)
	{
		if (IsValid(ItemPair.Value))
		{
			int32 ItemKey = StaticCast<int32>(ItemPair.Key);
			AddEquipmentItemToSlot(ItemPair.Value, ItemKey);
		}
	}
}

int32 UCharacterEquipmentComponent::GetAvailableAmunitionForCurrentWeapon() const
{
	check(IsValid(CurrentRangeWeapon));
	uint32 AmmoType = StaticCast<uint32>(GetCurrentRangeWeapon()->GetAmmoType());
	return AmunitionArray[AmmoType];
}

int32 UCharacterEquipmentComponent::GetGrenadesTotalAmount() const
{
	int32 Result = 0;
	if (MaxAmunitionAmount.Contains(EAmunitionType::FragGrenade))
	{
		Result = MaxAmunitionAmount[EAmunitionType::FragGrenade];
	}
	return Result;
}

void UCharacterEquipmentComponent::Server_AddAmmo_Implementation(EAmunitionType AmmoType, int32 AmmoCount)
{
	AddAmmoInternal(AmmoType, AmmoCount);
}

void UCharacterEquipmentComponent::Server_EquipItemInSlot_Implementation(EEquipmentSlots Slot)
{
	EquipItemInSlot(Slot);
}

void UCharacterEquipmentComponent::OnWeaponReloadComplete()
{
	ReloadAmmoInCurrentWeapon();
}

void UCharacterEquipmentComponent::OnWeaponUnloaded()
{
	int32 CurrentAmmo = CurrentRangeWeapon->GetAmmo();
	uint32 AmmoType = StaticCast<uint32>(CurrentRangeWeapon->GetAmmoType());
	if (GetOwnerRole() == ROLE_Authority)
	{
		AmunitionArray[AmmoType] += CurrentAmmo;
	}
	CurrentRangeWeapon->SetAmmo(0);
}

void UCharacterEquipmentComponent::OnGrenadeCountChanged() const
{
	if (OnGrenadeCountChangedEvent.IsBound())
	{
		OnGrenadeCountChangedEvent.Broadcast(CurrentGrenadesAmount, GetGrenadesTotalAmount());
	}
}

void UCharacterEquipmentComponent::OnCurrentWeaponAmmoChanged(int32 Ammo) const
{
	if (OnCurrentWeaponAmmoChangedEvent.IsBound())
	{
		OnCurrentWeaponAmmoChangedEvent.Broadcast(Ammo, GetAvailableAmunitionForCurrentWeapon());
	}
}

void UCharacterEquipmentComponent::OnEquipAnimationFinished()
{
	bIsEquipping = false;
	AttachCurrentItemToEquippedSocket();
}

void UCharacterEquipmentComponent::OnRep_CurrentEquippedSlot(EEquipmentSlots CurrentEquippedSlot_Old)
{
	EquipItemInSlot(CurrentEquippedSlot);
}

void UCharacterEquipmentComponent::OnRep_AmunitionArray()
{
	if (IsValid(CurrentRangeWeapon))
	{
		OnCurrentWeaponAmmoChanged(CurrentRangeWeapon->GetAmmo());
	}
}

void UCharacterEquipmentComponent::OnRep_ItemsArray()
{
	for (AEquipableItem* Item : ItemsArray)
	{
		if (IsValid(Item))
		{
			Item->UnEquip();
		}
	}
}

void UCharacterEquipmentComponent::OnRep_CurrentGrenades()
{
	OnGrenadeCountChanged();
}
