// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HomeworkBasePawn.generated.h"

UCLASS()
class HOMEWORK_API AHomeworkBasePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AHomeworkBasePawn();

	UPROPERTY(VisibleAnywhere)
	class UPawnMovementComponent* MyMovementComponent;

	UPROPERTY(VisibleAnywhere)
	class USphereComponent* CollisionComponent;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetInputForward() const { return InputForward; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetInputRight() const { return InputRight; }
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base pawn")
	float CollisionSphereRadius = 50.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base pawn")
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base pawn")
	class UCameraComponent* CameraComponent;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base pawn")
	class UArrowComponent* ArrowComponent;
#endif

public:	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float InValue);
	void MoveRight(float InValue);
	void Jump();

private:
	UFUNCTION()
	void OnBlendComplete();

	AActor* CurrentViewActor;

	float InputForward = 0.0f;
	float InputRight = 0.0f;
};
