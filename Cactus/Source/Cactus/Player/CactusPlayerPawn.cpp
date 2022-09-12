// Copyright © Ljung 2022. All rights reserved. https://ljung.dev/.


#include "CactusPlayerPawn.h"

#include "EnhancedInputComponent.h"
#include "SimpleMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

ACactusPlayerPawn::ACactusPlayerPawn(): CameraSensitivity(FVector2D(180.0f, 90.0f)),
                                        CameraPitchRange(FVector2D(-80.0f, 80.0f))
{
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collider"));
	SetRootComponent(CapsuleComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(CapsuleComponent);

	MovementComponent = CreateDefaultSubobject<USimpleMovementComponent>(TEXT("Movement"));
}

void ACactusPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (PlayerEnhancedInputComponent)
	{
		PlayerEnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this,
		                                         &ACactusPlayerPawn::OnInput_Move);
		PlayerEnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this,
		                                         &ACactusPlayerPawn::OnInput_Look);
		PlayerEnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this,
		                                         &ACactusPlayerPawn::OnInput_Jump);
	}
}

void ACactusPlayerPawn::OnInput_Move(const FInputActionValue& Value)
{
	const FVector2D RawInput = Value.Get<FVector2D>();
	const FVector ForwardInput = RawInput.X * GetActorForwardVector();
	const FVector RightInput = RawInput.Y * GetActorRightVector();
	AddMovementInput(ForwardInput + RightInput);
}

void ACactusPlayerPawn::OnInput_Look(const FInputActionValue& Value)
{
	const FVector2D VectorValue = Value.Get<FVector2D>() * CameraSensitivity * GetWorld()->GetDeltaSeconds();

	// Update pitch
	const FRotator& CameraRotation = CameraComponent->GetComponentRotation();
	const float NewPitch = FMath::Clamp(CameraRotation.Pitch + VectorValue.Y, CameraPitchRange.X, CameraPitchRange.Y);
	const FRotator NewCameraRotator = FRotator(NewPitch, CameraRotation.Yaw, CameraRotation.Roll);
	CameraComponent->SetWorldRotation(NewCameraRotator);

	// Update yaw
	const FRotator ActorRotationDelta = FRotator(0, VectorValue.X, 0);
	AddActorWorldRotation(ActorRotationDelta);
}

void ACactusPlayerPawn::OnInput_Jump()
{
	MovementComponent->Jump();
}
