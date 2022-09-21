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
	bool CheckForGround(FHitResult& OutHit, const float Height = 5.0f) const;

	bool Move(FHitResult& OutInitialHit, const float DeltaTime);
	void StepDown();

	void DoMovement_Walking(const float DeltaTime);
	void DoMovement_Falling(const float DeltaTime);
};
