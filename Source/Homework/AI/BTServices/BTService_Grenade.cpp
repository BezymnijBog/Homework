// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_Grenade.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/HWBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

UBTService_Grenade::UBTService_Grenade()
{
	NodeName = "ThrowGrenade";
}

void UBTService_Grenade::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!IsValid(AIController) || !IsValid(Blackboard) || bIsOnCooldown)
	{
		return;
	}

	AHWBaseCharacter* Character = Cast<AHWBaseCharacter>(AIController->GetPawn());
	if (!IsValid(Character))
	{
		return;
	}

	const UCharacterEquipmentComponent* Equipment = Character->GetEquipmentComponent();
	if (Equipment->GetGrenadesAmount() <= 0)
	{
		return;
	}

	AActor* CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(TargetKey.SelectedKeyName));
	if (!IsValid(CurrentTarget))
	{
		return;
	}

	float DistanceSquared = FVector::DistSquared(CurrentTarget->GetActorLocation(), Character->GetActorLocation());
	if (DistanceSquared > FMath::Square(MaxThrowDistance) || DistanceSquared < FMath::Square(MinThrowDistance))
	{
		return;
	}

	bIsOnCooldown = true;
	Character->EquipPrimaryItem();
	GetWorld()->GetTimerManager().SetTimer(CooldownTimer, [this]() {bIsOnCooldown = false; }, CooldownTime, false);
}
