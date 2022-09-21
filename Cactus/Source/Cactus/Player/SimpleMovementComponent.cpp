// Copyright © Ljung 2022. All rights reserved. https://ljung.dev/.


#include "SimpleMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/PhysicsVolume.h"

USimpleMovementComponent::USimpleMovementComponent(): MoveSpeed(600.0f), JumpForce(420.0f), MaxCeilingStopAngle(5.0f),
                                                      MaxStepDownHeight(20.0f)
{
}

void USimpleMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	UpdatedCollider = Cast<UCapsuleComponent>(UpdatedComponent);
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

	switch (MovementState)
	{
	case Walking:
		DoMovement_Walking(DeltaTime);
		break;
	case Falling:
		DoMovement_Falling(DeltaTime);
		break;
	}

	UpdateComponentVelocity();
}

void USimpleMovementComponent::Jump()
{
	if (MovementState == Walking)
	{
		Velocity.Z += JumpForce;
		MovementState = Falling;
	}
}

FVector USimpleMovementComponent::GetDesiredInputMovement(const FVector InputVector) const
{
	const FVector& Input = InputVector.GetClampedToMaxSize2D(1.0f);
	const FVector Movement = Input * MoveSpeed;
	return Movement;
}

bool USimpleMovementComponent::CheckForGround(FHitResult& OutHit, const float Height) const
{
	if (!UpdatedCollider)
	{
		return false;
	}

	const FVector Offset = FVector::UpVector * Height;
	const FVector ColliderLocation = UpdatedCollider->GetComponentLocation();
	const FVector StartLocation = ColliderLocation;
	const FVector EndLocation = ColliderLocation - Offset;

	FCollisionQueryParams SweepParams;
	SweepParams.AddIgnoredActor(UpdatedCollider->GetOwner());

	FCollisionResponseParams ResponseParams;
	UpdatedCollider->InitSweepCollisionParams(SweepParams, ResponseParams);

	FCollisionShape SweepShape = UpdatedCollider->GetCollisionShape();
	const ECollisionChannel CollisionChannel = UpdatedCollider->GetCollisionObjectType();

	bool bDidHit = GetWorld()->SweepSingleByChannel(
		OutHit, StartLocation, EndLocation, FQuat::Identity, CollisionChannel, SweepShape, SweepParams,
		ResponseParams);

	return bDidHit;
}

bool USimpleMovementComponent::Move(FHitResult& OutInitialHit, const float DeltaTime)
{
	const FRotator& Rotation = UpdatedComponent->GetComponentRotation();
	const FVector MovementDelta = Velocity * DeltaTime;

	SafeMoveUpdatedComponent(MovementDelta, Rotation, true, OutInitialHit);

	if (OutInitialHit.IsValidBlockingHit())
	{
		HandleImpact(OutInitialHit, DeltaTime, MovementDelta);

		FHitResult Hit(OutInitialHit);
		SlideAlongSurface(MovementDelta, 1.0f - OutInitialHit.Time, OutInitialHit.Normal, Hit, true);

		return true;
	}
	return false;
}

void USimpleMovementComponent::StepDown()
{
	FHitResult GroundHit;
	CheckForGround(GroundHit, MaxStepDownHeight);

	if (GroundHit.IsValidBlockingHit())
	{
		const FVector Diff = GroundHit.Location - GroundHit.TraceStart;
		const FVector StepCorrection = FVector::UpVector * -FMath::Abs(Diff.Z);

		const FRotator& Rotation = UpdatedComponent->GetComponentRotation();
		SafeMoveUpdatedComponent(StepCorrection, Rotation, true, GroundHit);
	}
}

void USimpleMovementComponent::DoMovement_Walking(const float DeltaTime)
{
	const FVector InputVector = ConsumeInputVector();
	const FVector HorizontalMovement = GetDesiredInputMovement(InputVector);
	Velocity += HorizontalMovement;

	FHitResult Hit;
	Move(Hit, DeltaTime);

	StepDown();

	const bool bIsGrounded = CheckForGround(Hit);

	if (!bIsGrounded)
	{
		MovementState = Falling;
	}
}

void USimpleMovementComponent::DoMovement_Falling(const float DeltaTime)
{
	const FVector InputVector = ConsumeInputVector();
	const FVector HorizontalMovement = GetDesiredInputMovement(InputVector);

	// Apply gravity
	const float GravityForce = GetGravityZ();
	const FVector VerticalMovement = FVector::UpVector * GravityForce * DeltaTime;
	const float TerminalVelocity = GetPhysicsVolume()->TerminalVelocity;

	Velocity += HorizontalMovement + VerticalMovement;
	Velocity.Z = FMath::Max(Velocity.Z, -TerminalVelocity);

	FHitResult Hit;
	bool bDidHit = Move(Hit, DeltaTime);

	// Check if we hit ceiling
	if (bDidHit && Velocity.Z > 0)
	{
		const float Dot = Hit.ImpactNormal.Dot(-FVector::UpVector);
		const float Angle = FMath::Acos(Dot);
		const float StopAngleInRadians = FMath::DegreesToRadians(MaxCeilingStopAngle);
		if (Angle <= StopAngleInRadians)
		{
			Velocity.Z = 0;
		}
	}

	const bool bIsGrounded = CheckForGround(Hit);
	if (bIsGrounded && Velocity.Z <= 0)
	{
		MovementState = Walking;
		Velocity.Z = 0;
	}
}
