// Copyright © Ljung 2022. All rights reserved. https://ljung.dev/.


#include "SimpleMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/PhysicsVolume.h"

USimpleMovementComponent::USimpleMovementComponent(): MoveSpeed(600.0f), JumpForce(420.0f), MaxCeilingStopAngle(5.0f),
                                                      MaxStepUpHeight(20.0f),
                                                      MinStepUpSteepness(10.0f),
                                                      MaxStepDownHeight(20.0f),
                                                      GroundActor(nullptr),
                                                      GroundActorLastYaw(0.0f)
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
	UpdateGroundActor();
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

bool USimpleMovementComponent::SweepWithCollider(FHitResult& OutHit, FVector StartLocation, FVector EndLocation) const
{
	if (!UpdatedCollider)
	{
		return false;
	}

	FCollisionQueryParams SweepParams;
	SweepParams.AddIgnoredActor(UpdatedCollider->GetOwner());

	FCollisionResponseParams ResponseParams;
	UpdatedCollider->InitSweepCollisionParams(SweepParams, ResponseParams);

	const FCollisionShape SweepShape = UpdatedCollider->GetCollisionShape();
	const ECollisionChannel CollisionChannel = UpdatedCollider->GetCollisionObjectType();

	bool bDidHit = GetWorld()->SweepSingleByChannel(
		OutHit, StartLocation, EndLocation, FQuat::Identity, CollisionChannel, SweepShape, SweepParams,
		ResponseParams);

	return bDidHit;
}

bool USimpleMovementComponent::CheckForGround(FHitResult& OutHit, const float Height) const
{
	const FVector Offset = FVector::UpVector * Height;
	const FVector ColliderLocation = UpdatedCollider->GetComponentLocation();
	const FVector StartLocation = ColliderLocation;
	const FVector EndLocation = ColliderLocation - Offset;

	return SweepWithCollider(OutHit, StartLocation, EndLocation);
}

void USimpleMovementComponent::UpdateGroundActor()
{
	FHitResult GroundHit;
	const bool bDidHit = CheckForGround(GroundHit);

	if (bDidHit)
	{
		GroundActor = GroundHit.GetActor();
		GroundActorLastYaw = GroundActor->GetActorRotation().Yaw;
	}
	else
	{
		GroundActor = nullptr;
	}
}

void USimpleMovementComponent::AdjustFromGroundMovement(const float DeltaTime)
{
	if (GroundActor)
	{
		FRotator NewRotation = UpdatedCollider->GetComponentRotation();
		FVector GroundActorLocationDelta = GroundActor->GetVelocity() * DeltaTime;
		const float GroundActorCurrentYaw = GroundActor->GetActorRotation().Yaw;
		const float GroundActorYawDelta = GroundActorCurrentYaw - GroundActorLastYaw;

		if (FMath::Abs(GroundActorYawDelta) > 0.0f)
		{
			NewRotation.Yaw += GroundActorYawDelta;

			FVector SelfLocationRelativeToGroundActor = UpdatedCollider->GetComponentLocation() - GroundActor->
				GetActorLocation();
			SelfLocationRelativeToGroundActor.Z = 0;

			const FRotator DeltaRotator = FRotator::MakeFromEuler(FVector(0, 0, GroundActorYawDelta));
			const FVector OffsetRelativeToGroundActor = DeltaRotator.
				RotateVector(SelfLocationRelativeToGroundActor);
			const FVector RotationOffsetRelativeToSelf = OffsetRelativeToGroundActor -
				SelfLocationRelativeToGroundActor;
			GroundActorLocationDelta += RotationOffsetRelativeToSelf;
		}


		MoveUpdatedComponent(GroundActorLocationDelta, NewRotation, false);
	}
}

bool USimpleMovementComponent::Move(FHitResult& OutInitialHit, FVector& OutMovementDelta, const float DeltaTime)
{
	const FRotator& Rotation = UpdatedComponent->GetComponentRotation();
	OutMovementDelta = Velocity * DeltaTime;

	SafeMoveUpdatedComponent(OutMovementDelta, Rotation, true, OutInitialHit);

	if (OutInitialHit.IsValidBlockingHit())
	{
		HandleImpact(OutInitialHit, DeltaTime, OutMovementDelta);
		return true;
	}

	return false;
}

void USimpleMovementComponent::Slide(const FVector MovementDelta, const FHitResult& Hit)
{
	FHitResult UnusedHit(Hit);
	SlideAlongSurface(MovementDelta, 1.0f - Hit.Time, Hit.Normal, UnusedHit, true);
}

bool USimpleMovementComponent::IsWithinStepUpSteepness(const FHitResult& Hit) const
{
	const float Dot = Hit.ImpactNormal | FVector::UpVector;
	const float MaxStepUpWallAngleRad = FMath::DegreesToRadians(MinStepUpSteepness);
	const float MaxDotValue = FMath::Sin(MaxStepUpWallAngleRad);
	const bool bIsValidAngle = Dot <= MaxDotValue;
	return bIsValidAngle;
}

bool USimpleMovementComponent::CanStepUp(const FHitResult& Hit,
                                         FVector& OutStepUpMovementDelta) const
{
	OutStepUpMovementDelta = FVector::ZeroVector;

	if (!Hit.IsValidBlockingHit())
	{
		return false;
	}

	const bool bIsValidAngle = IsWithinStepUpSteepness(Hit);
	if (!bIsValidAngle)
	{
		return false;
	}

	const FVector ColliderLocation = UpdatedCollider->GetComponentLocation();
	const FVector OriginalMovementDelta = Hit.TraceEnd - Hit.TraceStart;
	const FVector DesiredLocation = ColliderLocation + OriginalMovementDelta;
	const FVector StepUpOffset = FVector::UpVector * (MaxStepUpHeight + 0.1f);
	const FVector MaxStepUpLocation = DesiredLocation + StepUpOffset;

	FHitResult StepUpHit;
	const bool bDidHit = SweepWithCollider(StepUpHit, MaxStepUpLocation, DesiredLocation);
	const bool bCanStepUp = bDidHit && !StepUpHit.bStartPenetrating;

	if (bCanStepUp)
	{
		OutStepUpMovementDelta = StepUpHit.Location - ColliderLocation;
	}

	return bCanStepUp;
}

void USimpleMovementComponent::StepUp(const FVector& StepUpMovementDelta)
{
	FHitResult StepUpMoveHit;
	SafeMoveUpdatedComponent(StepUpMovementDelta, UpdatedCollider->GetComponentRotation(), false,
	                         StepUpMoveHit, ETeleportType::TeleportPhysics);
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
	FVector MovementDelta;
	bool bDidHit = Move(Hit, MovementDelta, DeltaTime);


	if (bDidHit)
	{
		FVector OutStepUpMovementDelta;
		bool bCanStepUp = CanStepUp(Hit, OutStepUpMovementDelta);

		if (bCanStepUp)
		{
			StepUp(OutStepUpMovementDelta);
		}
		else
		{
			const bool bIsWithinStepUpSteepness = IsWithinStepUpSteepness(Hit);
			if (bIsWithinStepUpSteepness)
			{
				Hit.Normal = Hit.Normal.GetSafeNormal2D();
			}

			Slide(MovementDelta, Hit);
		}
	}

	StepDown();

	AdjustFromGroundMovement(DeltaTime);

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
	FVector MovementDelta;
	bool bDidHit = Move(Hit, MovementDelta, DeltaTime);

	if (bDidHit)
	{
		Slide(MovementDelta, Hit);
	}

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
