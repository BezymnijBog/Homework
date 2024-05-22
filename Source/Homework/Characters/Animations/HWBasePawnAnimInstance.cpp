// Fill out your copyright notice in the Description page of Project Settings.


#include "HWBasePawnAnimInstance.h"

#include "GameFramework/PawnMovementComponent.h"
#include "Homework/Characters/HomeworkBasePawn.h"

void UHWBasePawnAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<AHomeworkBasePawn>(), TEXT("UHWBasePawnAnimInstance::NativeBeginPlay() can work only with HomeworkBasePawn"));
	CachedBasePawn = StaticCast<AHomeworkBasePawn*>(TryGetPawnOwner());
}

void UHWBasePawnAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!CachedBasePawn.IsValid())
	{
		return;
	}
	InputForward = CachedBasePawn->GetInputForward();
	InputRight = CachedBasePawn->GetInputRight();

	bIsInAir = CachedBasePawn->GetMovementComponent()->IsFalling();	
}
