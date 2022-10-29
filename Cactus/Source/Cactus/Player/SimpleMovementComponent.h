// Copyright © Ljung 2022. All rights reserved. https://ljung.dev/.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SimpleMovementComponent.generated.h"

UENUM()
enum EMovementState
{
	Walking,
	Falling
};

UCLASS()
class CACTUS_API USimpleMovementComponent final : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Movement")
	float MoveSpeed;

	UPROPERTY(EditAnywhere, Category="Movement")
	float JumpForce;

	UPROPERTY(EditAnywhere, Category="Movement")
	float MaxCeilingStopAngle;

	UPROPERTY(EditAnywhere, Category="Movement")
	float MaxStepUpHeight;

	UPROPERTY(EditAnywhere, Category="Movement")
	float MinStepUpSteepness;

	UPROPERTY(EditAnywhere, Category="Movement")
	float MaxStepDownHeight;

	USimpleMovementComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void Jump();

private:
	UPROPERTY()
	UCapsuleComponent* UpdatedCollider;

	EMovementState MovementState;

	FVector GetDesiredInputMovement(const FVector InputVector) const;
	bool SweepWithCollider(FHitResult& OutHit, FVector StartLocation, FVector EndLocation) const;
	bool CheckForGround(FHitResult& OutHit, const float Height = 5.0f) const;

	bool Move(FHitResult& OutInitialHit, FVector& OutMovementDelta, const float DeltaTime);
	void Slide(const FVector MovementDelta, const FHitResult& Hit);

	bool IsWithinStepUpSteepness(const FHitResult& Hit) const;
	bool CanStepUp(const FHitResult& Hit, FVector& OutStepUpMovementDelta) const;
	void StepUp(const FVector& StepUpMovementDelta);
	void StepDown();

	void DoMovement_Walking(const float DeltaTime);
	void DoMovement_Falling(const float DeltaTime);
};
