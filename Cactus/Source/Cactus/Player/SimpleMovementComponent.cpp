// Copyright © Ljung 2022. All rights reserved. https://ljung.dev/.


#include "SimpleMovementComponent.h"

#include "GameFramework/PhysicsVolume.h"

USimpleMovementComponent::USimpleMovementComponent(): MoveSpeed(600.0f)
{
}

void USimpleMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	if (!PawnOwner || !UpdatedComponent)
	{
		return;
	}

	// Reset velocity
	Velocity.X = 0;
	Velocity.Y = 0;

	// Calculate force
	const FVector& Input = ConsumeInputVector().GetClampedToMaxSize2D(1.0f);
	const FVector HorizontalMovement = Input * MoveSpeed;

	// Apply gravity
	const float GravityForce = GetGravityZ();
	const FVector VerticalMovement = FVector::UpVector * GravityForce * DeltaTime;

	// Move
	const FRotator& Rotation = UpdatedComponent->GetComponentRotation();
	const float TerminalVelocity = GetPhysicsVolume()->TerminalVelocity;
	Velocity += HorizontalMovement + VerticalMovement;
	Velocity.Z = FMath::Max(Velocity.Z, -TerminalVelocity);
	const FVector MovementDelta = Velocity * DeltaTime;

	FHitResult Hit;
	SafeMoveUpdatedComponent(MovementDelta, Rotation, true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		Velocity.Z = 0;
		HandleImpact(Hit, DeltaTime, MovementDelta);
		SlideAlongSurface(MovementDelta, 1.0f - Hit.Time, Hit.Normal, Hit, true);
	}

	UpdateComponentVelocity();
}
