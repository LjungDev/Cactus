// Copyright © Ljung 2022. All rights reserved. https://ljung.dev/.


#include "SimpleMovementComponent.h"

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
	const FVector DesiredInputForce = Input * MoveSpeed;
	Velocity += DesiredInputForce;
	const FVector MovementDelta = Velocity * DeltaTime;

	// Move
	const FRotator& Rotation = UpdatedComponent->GetComponentRotation();

	FHitResult Hit;
	SafeMoveUpdatedComponent(MovementDelta, Rotation, true, Hit);
	
	if (Hit.IsValidBlockingHit())
	{
		HandleImpact(Hit, DeltaTime, MovementDelta);
		SlideAlongSurface(MovementDelta, 1.0f - Hit.Time, Hit.Normal, Hit, true);
	}
	
	UpdateComponentVelocity();
}
