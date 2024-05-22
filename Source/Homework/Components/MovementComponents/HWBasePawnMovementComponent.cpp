// Fill out your copyright notice in the Description page of Project Settings.


#include "HWBasePawnMovementComponent.h"

void UHWBasePawnMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	if (ShouldSkipUpdate(DeltaTime))
	{
		return;
	}
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const FVector PendingInput = GetPendingInputVector().GetClampedToMaxSize(1.0f);
	Velocity = PendingInput * MaxSpeed;
	ConsumeInputVector();

	if (bEnableGravity)
	{
		FHitResult HitResult;
		FVector StartPoint = UpdatedComponent->GetRelativeLocation();
		float SphereRadius = 50.0f;
		float TraceDepth = 1.0f;
		FVector EndPoint = StartPoint - TraceDepth * FVector::UpVector;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(GetOwner());

		bool bWasFalling = bIsFalling;
		FCollisionShape Sphere = FCollisionShape::MakeSphere(SphereRadius);
		bIsFalling = !GetWorld()->SweepSingleByChannel(HitResult, StartPoint, EndPoint, FQuat::Identity, ECC_Visibility, Sphere, CollisionParams);
		if (bIsFalling)
		{
			VerticalVelocity += GetGravityZ() * FVector::UpVector * DeltaTime;
		}
		else if (bWasFalling && VerticalVelocity.Z < 0.0)
		{
			VerticalVelocity = FVector::ZeroVector;
		}
	}
	Velocity += VerticalVelocity;
	const FVector Shift = Velocity * DeltaTime;
	if (!Shift.IsNearlyZero())
	{
		const FQuat Rot = UpdatedComponent->GetComponentQuat();
		FHitResult Hit(1.f);
		SafeMoveUpdatedComponent(Shift, Rot, true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			HandleImpact(Hit, DeltaTime, Shift);
			// Try to slide the remaining distance along the surface.
			SlideAlongSurface(Shift, 1.f - Hit.Time, Hit.Normal, Hit, true);
		}
	}
	UpdateComponentVelocity();
	
}

void UHWBasePawnMovementComponent::JumpStart()
{
	VerticalVelocity = InitialJumpSpeed * FVector::UpVector;
}

bool UHWBasePawnMovementComponent::IsFalling() const
{
	return bIsFalling;
}
