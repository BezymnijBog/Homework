// Fill out your copyright notice in the Description page of Project Settings.


#include "HomeworkBasePawn.h"

#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/SpringArmComponent.h"
#include "Homework/Components/MovementComponents/HWBasePawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AHomeworkBasePawn::AHomeworkBasePawn()
{
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->SetSphereRadius(CollisionSphereRadius);
	CollisionComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	RootComponent = CollisionComponent;

	MyMovementComponent = CreateDefaultSubobject<UPawnMovementComponent, UHWBasePawnMovementComponent>(TEXT("Movement component"));
	MyMovementComponent->SetUpdatedComponent(CollisionComponent);

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring arm"));
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->SetupAttachment(RootComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);

#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	ArrowComponent->SetupAttachment(RootComponent);
#endif

}

void AHomeworkBasePawn::BeginPlay()
{
	Super::BeginPlay();
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	CurrentViewActor = CameraManager->GetViewTarget();
	CameraManager->OnBlendComplete().AddUFunction(this, FName("OnBlendComplete"));
}

// Called to bind functionality to input
void AHomeworkBasePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Turn", this, &Super::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &Super::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("MoveForward", this, &AHomeworkBasePawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHomeworkBasePawn::MoveRight);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AHomeworkBasePawn::Jump);

}

void AHomeworkBasePawn::MoveForward(float InValue)
{
	InputForward = InValue;
	if (InValue != 0.0f)
	{
		AddMovementInput(CurrentViewActor->GetActorForwardVector(), InValue);
	}
}

void AHomeworkBasePawn::MoveRight(float InValue)
{
	InputRight = InValue;
	if (InValue != 0.0f)
	{
		AddMovementInput(CurrentViewActor->GetActorRightVector(), InValue);
	}
}

void AHomeworkBasePawn::Jump()
{
	checkf(MyMovementComponent->IsA<UHWBasePawnMovementComponent>(), TEXT("AHomeworkBasePawn::Jump works only with UHWBasePawnMovementComponent"));
	UHWBasePawnMovementComponent* BaseMovement = StaticCast<UHWBasePawnMovementComponent*>(MyMovementComponent);
	BaseMovement->JumpStart();
}

void AHomeworkBasePawn::OnBlendComplete()
{
	CurrentViewActor = GetController()->GetViewTarget();
	UE_LOG(LogTemp, Log, TEXT("AHomeworkBasePawn::OnBlendComplete CurrentViewActor is %s"), *CurrentViewActor->GetName());
}

