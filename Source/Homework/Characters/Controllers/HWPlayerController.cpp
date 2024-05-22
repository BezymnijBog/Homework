// Fill out your copyright notice in the Description page of Project Settings.


#include "HWPlayerController.h"


#include "Blueprint/UserWidget.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "GameFramework/PlayerInput.h"
#include "Homework/Characters/HWBaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/SaveSubsystem/SaveSubsystem.h"
#include "UI/Widget/AmmoWidget.h"
#include "UI/Widget/PlayerHUDWidget.h"
#include "UI/Widget/ReticleWidget.h"
#include "UI/Widget/WidgetPlayerAttributes.h"

void AHWPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	CachedBaseCharacter = Cast<AHWBaseCharacter>(InPawn);
	if (IsValid(InPawn) && IsLocalController())
	{
		CreateAndInitializeWidgets();
		CachedBaseCharacter->OnInteractiveObjectFound.BindUObject(this, &AHWPlayerController::OnInteractiveObjectFound);
	}
}

void AHWPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAxis("MoveForward", this, &AHWPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AHWPlayerController::MoveRight);
	InputComponent->BindAxis("Turn", this, &AHWPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &AHWPlayerController::LookUp);
	InputComponent->BindAxis("TurnAtRate", this, &AHWPlayerController::TurnAtRate);
	InputComponent->BindAxis("LookUpAtRate", this, &AHWPlayerController::LookUpAtRate);
	InputComponent->BindAxis("SwimForward", this, &AHWPlayerController::SwimForward);
	InputComponent->BindAxis("SwimRight", this, &AHWPlayerController::SwimRight);
	InputComponent->BindAxis("SwimUp", this, &AHWPlayerController::SwimUp);
	InputComponent->BindAxis("ClimbLadderUp", this, &AHWPlayerController::ClimbLadderUp);
	InputComponent->BindAction("InteractWithLadder", EInputEvent::IE_Pressed, this, &AHWPlayerController::InteractWithLadder);
	InputComponent->BindAction("ZiplineInteraction", EInputEvent::IE_Pressed, this, &AHWPlayerController::ZiplineInteraction);
	InputComponent->BindAction("Mantle", EInputEvent::IE_Pressed, this, &AHWPlayerController::Mantle);
	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AHWPlayerController::Jump);
	InputComponent->BindAction("Slide", EInputEvent::IE_Pressed, this, &AHWPlayerController::StartSlide);
	InputComponent->BindAction("Slide", EInputEvent::IE_Released, this, &AHWPlayerController::StopSlide);
	InputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &AHWPlayerController::ChangeCrouchState);
	InputComponent->BindAction("Prone", EInputEvent::IE_Pressed, this, &AHWPlayerController::ChangeProneState);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &AHWPlayerController::StartSprint);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &AHWPlayerController::StopSprint);
	InputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &AHWPlayerController::PlayerStartFire);
	InputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &AHWPlayerController::PlayerStopFire);
	InputComponent->BindAction("Aim", EInputEvent::IE_Pressed, this, &AHWPlayerController::StartAim);
	InputComponent->BindAction("Aim", EInputEvent::IE_Released, this, &AHWPlayerController::StopAim);
	InputComponent->BindAction("Reload", EInputEvent::IE_Pressed, this, &AHWPlayerController::Reload);
	InputComponent->BindAction("NextItem", EInputEvent::IE_Pressed, this, &AHWPlayerController::NexItem);
	InputComponent->BindAction("PreviousItem", EInputEvent::IE_Pressed, this, &AHWPlayerController::PreviousItem);
	InputComponent->BindAction("EquipPrimaryItem", EInputEvent::IE_Pressed, this, &AHWPlayerController::EquipPrimaryItem);
	InputComponent->BindAction("PrimaryMeleeAttack", EInputEvent::IE_Pressed, this, &AHWPlayerController::PrimaryMeleeAttack);
	InputComponent->BindAction("SecondaryMeleeAttack", EInputEvent::IE_Pressed, this, &AHWPlayerController::SecondaryMeleeAttack);
	InputComponent->BindAction(ActionInteract, EInputEvent::IE_Pressed, this, &AHWPlayerController::Interact);
	InputComponent->BindAction("UseInventory", EInputEvent::IE_Pressed, this, &AHWPlayerController::UseInventory);
	InputComponent->BindAction("ConfirmWheelSelection", EInputEvent::IE_Pressed, this, &AHWPlayerController::ConfirmWeaponWheelSelection);
	InputComponent->BindAction("QuickSave", EInputEvent::IE_Pressed, this, &AHWPlayerController::QuickSaveGame);
	InputComponent->BindAction("QuickLoad", EInputEvent::IE_Pressed, this, &AHWPlayerController::QuickLoadGame);
	InputComponent->BindAction("SwitchWeaponMode", EInputEvent::IE_Pressed, this, &AHWPlayerController::SwitchWeaponMode);

	FInputBinding& ToggleMenuBinding = InputComponent->BindAction("ToggleMenu", IE_Pressed, this, &AHWPlayerController::ToggleMainMenu);
	ToggleMenuBinding.bExecuteWhenPaused = true;
}

void AHWPlayerController::MoveForward(float InValue)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->MoveForward(InValue);
	}
}

void AHWPlayerController::MoveRight(float InValue)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->MoveRight(InValue);
	}
}

void AHWPlayerController::Turn(float InValue)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Turn(InValue);
	}
}

void AHWPlayerController::LookUp(float InValue)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->LookUp(InValue);
	}
}

void AHWPlayerController::TurnAtRate(float InValue)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->TurnAtRate(InValue);
	}
}

void AHWPlayerController::LookUpAtRate(float InValue)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->LookUpAtRate(InValue);
	}
}

void AHWPlayerController::SwimForward(float InValue)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimForward(InValue);
	}
}

void AHWPlayerController::SwimRight(float InValue)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimRight(InValue);
	}
}

void AHWPlayerController::SwimUp(float InValue)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimUp(InValue);
	}
}

void AHWPlayerController::ClimbLadderUp(float InValue)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ClimbLadderUp(InValue);
	}
}

void AHWPlayerController::InteractWithLadder()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->InteractWithLadder();
	}
}

void AHWPlayerController::Mantle()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Mantle();
	}
}

void AHWPlayerController::Jump()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Jump();
	}
}

void AHWPlayerController::ZiplineInteraction()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ZiplineInteraction();
	}
}

void AHWPlayerController::StartSlide()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartSlide();
	}
}

void AHWPlayerController::StopSlide()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopSlide();
	}
}

void AHWPlayerController::ChangeCrouchState()
{
	if (CachedBaseCharacter.IsValid() && !CachedBaseCharacter->bIsCrouched && !CachedBaseCharacter->IsProne())
	{
		CachedBaseCharacter->ChangeCrouchState();
	}
}

void AHWPlayerController::ChangeProneState()
{
	if (CachedBaseCharacter.IsValid() && (CachedBaseCharacter->bIsCrouched || CachedBaseCharacter->IsProne()))
	{
		CachedBaseCharacter->ChangeProneState();
	}
}

void AHWPlayerController::StartSprint()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartSprint();
	}
}

void AHWPlayerController::StopSprint()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopSprint();
	}
}

void AHWPlayerController::PlayerStartFire()
{
	Super::StartFire();
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartFire();
	}
}

void AHWPlayerController::PlayerStopFire()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopFire();
	}
}

void AHWPlayerController::StartAim()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartAim();
	}
}

void AHWPlayerController::StopAim()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopAim();
	}
}

void AHWPlayerController::Reload()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Reload();
	}
}

void AHWPlayerController::NexItem()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->NexItem();
	}
}

void AHWPlayerController::PreviousItem()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->PreviousItem();
	}
}

void AHWPlayerController::EquipPrimaryItem()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->EquipPrimaryItem();
	}
}

void AHWPlayerController::PrimaryMeleeAttack()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->PrimaryMeleeAttack();
	}
}

void AHWPlayerController::SecondaryMeleeAttack()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SecondaryMeleeAttack();
	}
}

void AHWPlayerController::Interact()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Interact();
	}
}

void AHWPlayerController::UseInventory()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->UseInventory(this);
	}
}

void AHWPlayerController::ConfirmWeaponWheelSelection()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ConfirmWeaponSelection();
	}
}

void AHWPlayerController::QuickSaveGame()
{
	USaveSubsystem* SaveSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<USaveSubsystem>();
	SaveSubsystem->SaveGame();
}

void AHWPlayerController::QuickLoadGame()
{
	USaveSubsystem* SaveSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<USaveSubsystem>();
	SaveSubsystem->LoadLastGame();
}

void AHWPlayerController::SwitchWeaponMode()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwitchWeaponMode();
	}
}

void AHWPlayerController::ToggleMainMenu()
{
	if (!IsValid(MainMenuWidget) || !IsValid(PlayerHUDWidget))
	{
		return;
	}
	if (MainMenuWidget->IsVisible())
	{
		MainMenuWidget->RemoveFromParent();
		PlayerHUDWidget->AddToViewport();
		SetInputMode(FInputModeGameOnly{});
		SetPause(false);
		bShowMouseCursor = false;
	}
	else
	{
		MainMenuWidget->AddToViewport();
		PlayerHUDWidget->RemoveFromParent();
		SetInputMode(FInputModeGameAndUI{});
		SetPause(true);
		bShowMouseCursor = true;
	}
}

void AHWPlayerController::CreateAndInitializeWidgets()
{
	if (!IsValid(PlayerHUDWidget) && IsValid(PlayerHUDWidgetClass))
	{
		PlayerHUDWidget = CreateWidget<UPlayerHUDWidget>(GetWorld(), PlayerHUDWidgetClass);
		PlayerHUDWidget->AddToViewport();
	}

	if (!IsValid(MainMenuWidget) && IsValid(MainMenuWidgetClass))
	{
		MainMenuWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);
	}
	
	if (IsValid(PlayerHUDWidget) && CachedBaseCharacter.IsValid())
	{
		UCharacterEquipmentComponent* CharacterEquipment = CachedBaseCharacter->GetEquipmentComponent_Mutable();
		UCharacterAttributesComponent* CharacterAttributes = CachedBaseCharacter->GetCharacterAttributes_Mutable();
		UReticleWidget* ReticleWidget = PlayerHUDWidget->GetReticleWidget();
		if (IsValid(ReticleWidget))
		{
			CachedBaseCharacter->OnAimingStateChanged.AddUFunction(ReticleWidget, FName("OnAimingStateChanged"));
			CharacterEquipment->OnEquippedItemChanged.AddUFunction(ReticleWidget, FName("OnEquippedItemChanged"));
		}

		UAmmoWidget* AmmoWidget = PlayerHUDWidget->GetAmmoWidget();
		if (IsValid(AmmoWidget))
		{
			CharacterEquipment->OnCurrentWeaponAmmoChangedEvent.AddUFunction(AmmoWidget, FName("UpdateAmmoCount"));
			CharacterEquipment->OnGrenadeCountChangedEvent.AddUFunction(AmmoWidget, FName("UpdateGrenadeCount"));
		}

		UWidgetPlayerAttributes* AttributesWidget = PlayerHUDWidget->GetAttributesWidget();
		if (IsValid(AttributesWidget))
		{
			CharacterAttributes->OnAttributeChangedDelegate.AddUFunction(AttributesWidget, FName("OnAttributesChanged"));
		}
	}

	SetInputMode(FInputModeGameOnly{});
	bShowMouseCursor = false;
}

void AHWPlayerController::OnInteractiveObjectFound(FName ActionName)
{
	if (!IsValid(PlayerHUDWidget))
	{
		return;
	}

	TArray<FInputActionKeyMapping> ActionKeys = PlayerInput->GetKeysForAction(ActionName);
	const bool bHasAnyKeys = ActionKeys.Num() != 0;
	if (bHasAnyKeys)
	{
		// take only first key for interaction
		FName ActionKey = ActionKeys[0].Key.GetFName();
		PlayerHUDWidget->SetHighlightInteractiveActionText(ActionKey);
	}
	PlayerHUDWidget->SetHighlightInteractiveVisibility(bHasAnyKeys);
}
