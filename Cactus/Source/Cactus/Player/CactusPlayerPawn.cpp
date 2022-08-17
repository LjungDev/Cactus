// Copyright © Ljung 2022. All rights reserved. https://ljung.dev/.


#include "CactusPlayerPawn.h"

#include "SimpleMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

ACactusPlayerPawn::ACactusPlayerPawn()
{
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collider"));
	SetRootComponent(CapsuleComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(CapsuleComponent);

	MovementComponent = CreateDefaultSubobject<USimpleMovementComponent>(TEXT("Movement"));
}
