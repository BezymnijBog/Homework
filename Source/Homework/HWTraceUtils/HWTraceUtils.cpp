#include "HWTraceUtils.h"

#include "DrawDebugHelpers.h"

bool HWTraceUtils::SweepCapsuleSingleByChanel(const UWorld* World, FHitResult& OutHit,
	const FVector& Start, const FVector& End, float CapsuleRadius, float CapsuleHalfHeight,
	const FQuat& Rot, ECollisionChannel TraceChannel, const FCollisionQueryParams& Params,
	const FCollisionResponseParams& ResponseParam, bool bDrawDebug, float DrawTime, FColor TraceColor, FColor HitColor)
{
	bool bResult = false;
	const FCollisionShape CollisionCapsule = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	bResult |= World->SweepSingleByChannel(OutHit, Start, End, Rot, TraceChannel, CollisionCapsule, Params, ResponseParam);
#if ENABLE_DRAW_DEBUG
  	if (bDrawDebug)
	{
		DrawDebugCapsule(World, Start, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, TraceColor, false, DrawTime);
		DrawDebugLine(World, Start, End, TraceColor, false, DrawTime);
		DrawDebugCapsule(World, End, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, TraceColor, false, DrawTime);
		if (bResult)
		{
			DrawDebugCapsule(World, OutHit.Location, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, HitColor, false, DrawTime);
			DrawDebugPoint(World, OutHit.ImpactPoint, 10.f, HitColor, false, DrawTime);
		}
	}
#endif

	return bResult;
}

bool HWTraceUtils::SweepSphereSingleByChanel(const UWorld* World, FHitResult& OutHit, const FVector& Start,
	const FVector& End, float Radius, ECollisionChannel TraceChannel, const FCollisionQueryParams& Params,
	const FCollisionResponseParams& ResponseParam, bool bDrawDebug, float DrawTime, FColor TraceColor, FColor HitColor)
{
	bool bResult = false;
	const FCollisionShape CollisionCapsule = FCollisionShape::MakeSphere(Radius);
	bResult |= World->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, TraceChannel, CollisionCapsule, Params, ResponseParam);
#if ENABLE_DRAW_DEBUG
  	if (bDrawDebug)
	{
		const FVector DebugCenter = (Start + End) * 0.5f;
		const FVector TraceVector = End - Start;
		const float DebugDrawCapsuleHalfHeight = TraceVector.Size() * 0.5f;
		const FQuat DebugCapsuleRotation = FRotationMatrix::MakeFromZ(TraceVector).ToQuat();
		DrawDebugCapsule(World, DebugCenter, DebugDrawCapsuleHalfHeight, Radius, DebugCapsuleRotation, TraceColor, false, DrawTime);
		if (bResult)
		{
			DrawDebugSphere(World, OutHit.Location, Radius, 32, HitColor, false, DrawTime);
			DrawDebugPoint(World, OutHit.ImpactPoint, 10.f, HitColor, false, DrawTime);
		}
		
	}
#endif

	return bResult;
}

bool HWTraceUtils::OverlapCapsuleAnyByProfile(const UWorld* World, const FVector& Position, float CapsuleRadius,
	float CapsuleHalfHeight, const FQuat& Rot, FName ProfileName, const FCollisionQueryParams& QueryParams,
	bool bDrawDebug, float DrawTime, FColor HitColor)
{
	bool bResult = false;
	const FCollisionShape CollisionCapsule = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	bResult |= World->OverlapAnyTestByProfile(Position, Rot, ProfileName, CollisionCapsule, QueryParams);
#if ENABLE_DRAW_DEBUG
  	if (bDrawDebug && bResult)
	{
		DrawDebugCapsule(World, Position, CapsuleHalfHeight, CapsuleRadius, Rot, HitColor, false, DrawTime);
	}
#endif

	return bResult;
}

bool HWTraceUtils::OverlapCapsuleBlockingByProfile(const UWorld* World, const FVector& Position, float CapsuleRadius,
	float CapsuleHalfHeight, const FQuat& Rot, FName ProfileName, const FCollisionQueryParams& QueryParams,
	bool bDrawDebug, float DrawTime, FColor HitColor)
{
	bool bResult = false;
	const FCollisionShape CollisionCapsule = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	bResult |= World->OverlapBlockingTestByProfile(Position, Rot, ProfileName, CollisionCapsule, QueryParams);
#if ENABLE_DRAW_DEBUG
	if (bDrawDebug && bResult)
	{
		DrawDebugCapsule(World, Position, CapsuleHalfHeight, CapsuleRadius, Rot, HitColor, false, DrawTime);
	}
#endif

	return bResult;
}
