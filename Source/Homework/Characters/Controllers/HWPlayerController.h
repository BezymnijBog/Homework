// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HWPlayerController.generated.h"

class UPlayerHUDWidget;
/**
 * 
 */
UCLASS()
class HOMEWORK_API AHWPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetPawn(APawn* InPawn) override;

	void SetIgnoreCameraPitch(bool bIgnore) { bIgnoreCameraPitch = bIgnore; }
	bool GetIgnoreCameraPitch() const { return bIgnoreCameraPitch; }
	
protected:
	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UPlayerHUDWidget> PlayerHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

private:
	void MoveForward(float InValue);
	void MoveRight(float InValue);
	void Turn(float InValue);
	void LookUp(float InValue);
	void TurnAtRate(float InValue);
	void LookUpAtRate(float InValue);
	void SwimForward(float InValue);
	void SwimRight(float InValue);
	void SwimUp(float InValue);
	void ClimbLadderUp(float InValue);
	void InteractWithLadder();
	void Mantle();
	void Jump();
	void ZiplineInteraction();
	void StartSlide();
	void StopSlide();
	void ChangeCrouchState();
	void ChangeProneState();
	void StartSprint();
	void StopSprint();
	void PlayerStartFire();
	void PlayerStopFire();
	void StartAim();
	void StopAim();
	void Reload();
	void NexItem();
	void PreviousItem();
	void EquipPrimaryItem();
	void PrimaryMeleeAttack();
	void SecondaryMeleeAttack();
	void Interact();
	void UseInventory();
	void ConfirmWeaponWheelSelection();

	void QuickSaveGame();
	void QuickLoadGame();

	void SwitchWeaponMode();
	void ToggleMainMenu();
	
	void CreateAndInitializeWidgets();

	void OnInteractiveObjectFound(FName ActionName);
	
	TSoftObjectPtr<class AHWBaseCharacter> CachedBaseCharacter;

	UPROPERTY()
	UPlayerHUDWidget* PlayerHUDWidget = nullptr;
	UPROPERTY()
	UUserWidget* MainMenuWidget = nullptr;
	
	bool bIgnoreCameraPitch = false;
};
