// Copyright © Ljung 2022. All rights reserved. https://ljung.dev/.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CactusPlayerPawn.generated.h"

UCLASS()
class CACTUS_API ACactusPlayerPawn final : public APawn
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	class UCapsuleComponent* CapsuleComponent;

	UPROPERTY(EditDefaultsOnly)
	class UCameraComponent* CameraComponent;

	UPROPERTY(EditDefaultsOnly)
	class USimpleMovementComponent* MovementComponent;

	ACactusPlayerPawn();
};
