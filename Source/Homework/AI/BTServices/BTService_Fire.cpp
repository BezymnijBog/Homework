// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_Fire.h"

#include "AIController.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/HWBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

UBTService_Fire::UBTService_Fire()
{
    NodeName = "Fire";
}

void UBTService_Fire::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

    if (!IsValid(AIController) || !IsValid(Blackboard))
    {
        return;
    }

    AHWBaseCharacter* Character = Cast<AHWBaseCharacter>(AIController->GetPawn());
    if (!IsValid(Character))
    {
        return;
    }

    const UCharacterEquipmentComponent* Equipment = Character->GetEquipmentComponent();
    ARangeWeaponItem* RangeWeapon = Equipment->GetCurrentRangeWeapon();

    if (!IsValid(RangeWeapon))
    {
        return;
    }

    AActor* CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!IsValid(CurrentTarget))
    {
        Character->StopFire();
        return;
    }

    float DistanceSquared = FVector::DistSquared(CurrentTarget->GetActorLocation(), Character->GetActorLocation());
    if (DistanceSquared > FMath::Square(MaxFireDistance))
    {
        Character->StopFire();
        return;
    }

    if (!RangeWeapon->IsReloading() && !RangeWeapon->IsFiring())
    {
        Character->StartFire();
    }
}
