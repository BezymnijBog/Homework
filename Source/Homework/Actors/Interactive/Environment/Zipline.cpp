// Fill out your copyright notice in the Description page of Project Settings.


#include "Zipline.h"

#include "Components/CapsuleComponent.h"
#include "Homework/HomeworkTypes.h"

AZipline::AZipline()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Zipline Root"));
	
	FirstPoleStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("First Pole");
	FirstPoleStaticMeshComponent->SetupAttachment(RootComponent);
	FirstPoleStaticMeshComponent->SetCollisionProfileName(CollisionProfileNoCollision);
	FirstPoleStaticMeshComponent->SetGenerateOverlapEvents(false);
	
	SecondPoleStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Second Pole");
	SecondPoleStaticMeshComponent->SetupAttachment(RootComponent);
	SecondPoleStaticMeshComponent->SetCollisionProfileName(CollisionProfileNoCollision);
	SecondPoleStaticMeshComponent->SetGenerateOverlapEvents(false);
	
	CableStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Cable");
	CableStaticMeshComponent->SetupAttachment(RootComponent);
	CableStaticMeshComponent->SetCollisionProfileName(CollisionProfileNoCollision);
	CableStaticMeshComponent->SetGenerateOverlapEvents(false);

	InteractionVolume = CreateDefaultSubobject<UCapsuleComponent>("Interaction Volume");
	InteractionVolume->SetupAttachment(RootComponent);
	InteractionVolume->SetCollisionProfileName(CollisionProfilePawnInteractionVolume);
	InteractionVolume->SetGenerateOverlapEvents(true);
}

void AZipline::OnConstruction(const FTransform& Transform)
{
	const bool bIsValidObject = IsValid(FirstPoleStaticMeshComponent) && IsValid(SecondPoleStaticMeshComponent) && IsValid(CableStaticMeshComponent) && IsValid(InteractionVolume);
	if (!bIsValidObject)
	{
		return;
	}
	const FVector FirstPoleLocation(0.f, 0.f, PoleHeight * 0.5f);
	FirstPoleStaticMeshComponent->SetRelativeLocation(FirstPoleLocation);
	UStaticMesh* FirstPoleMesh = FirstPoleStaticMeshComponent->GetStaticMesh();
	UStaticMesh* SecondPoleMesh = SecondPoleStaticMeshComponent->GetStaticMesh();
	if (IsValid(FirstPoleMesh) && IsValid(SecondPoleMesh))
	{
		float FirstPoleHeight = FirstPoleMesh->GetBoundingBox().GetSize().Z;
		float SecondPoleHeight = SecondPoleMesh->GetBoundingBox().GetSize().Z;
		FirstPoleStaticMeshComponent->SetRelativeScale3D(FVector(1.f, 1.f, PoleHeight / FirstPoleHeight));
		SecondPoleStaticMeshComponent->SetRelativeScale3D(FVector(1.f, 1.f, PoleHeight / SecondPoleHeight));
	}
	FVector FirstPoleTop(0.f, 0.f, PoleHeight);
	FVector SecondPoleTop = SecondPoleStaticMeshComponent->GetRelativeLocation();
	SecondPoleTop.Z += PoleHeight * 0.5f;
	FRotator InitCableRotator(-90.f, 0.f, 0.f);
	FRotator CableRotation = (SecondPoleTop - FirstPoleTop).ToOrientationRotator();

	UStaticMesh* CableMesh = CableStaticMeshComponent->GetStaticMesh();
	if (IsValid(CableMesh))
	{
		float CableDefaultLength = CableMesh->GetBoundingBox().GetSize().Z;
		float CableRequiredLength = FVector::Dist(SecondPoleTop, FirstPoleTop);
		float CableRelativeScale = CableRequiredLength / CableDefaultLength;
		CableStaticMeshComponent->SetRelativeScale3D(FVector(1.f, 1.f, CableRelativeScale));
		GetInteractionCapsule()->SetCapsuleHalfHeight(CableRequiredLength * 0.5f);
		GetInteractionCapsule()->SetCapsuleRadius(InteractionCapsuleRadius);
	}
	CableStaticMeshComponent->SetRelativeLocation((FirstPoleTop + SecondPoleTop) * 0.5f);
	CableStaticMeshComponent->SetRelativeRotation(CableRotation + InitCableRotator);
	GetInteractionCapsule()->SetRelativeLocation((FirstPoleTop + SecondPoleTop) * 0.5f);
	GetInteractionCapsule()->SetRelativeRotation(CableRotation + InitCableRotator);
}

FVector AZipline::GetMoveDirection() const
{
	const FVector& FirstPoleLocation = FirstPoleStaticMeshComponent->GetRelativeLocation();
	const FVector& SecondPoleLocation = SecondPoleStaticMeshComponent->GetRelativeLocation();
	FVector Result = FirstPoleLocation - SecondPoleLocation;
	Result.Normalize();
	return  FirstPoleLocation.Z > SecondPoleLocation.Z ? -Result : Result;
}

FVector AZipline::GetClosestCablePoint(const FVector& Location) const
{
	const FVector CableDir = GetMoveDirection();
	FVector CableStart = GetActorLocation() + FVector::UpVector * PoleHeight;
	FVector Projection = CableStart + CableDir * FVector::DotProduct(CableDir, Location - CableStart);
	return Projection;
}

bool AZipline::IsDetachNeeded(const FVector& Location) const
{
	return (GetLowPole()->GetComponentLocation() - Location).Size2D() < DetachingDistance;
}

UCapsuleComponent* AZipline::GetInteractionCapsule() const
{
	return StaticCast<UCapsuleComponent*>(InteractionVolume);
}

UStaticMeshComponent* AZipline::GetLowPole() const
{
	if (FirstPoleStaticMeshComponent->GetComponentLocation().Z > SecondPoleStaticMeshComponent->GetComponentLocation().Z)
	{
		return SecondPoleStaticMeshComponent;
	}
	// else
	return FirstPoleStaticMeshComponent;
}
