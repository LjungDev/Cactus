// Copyright © Ljung 2022. All rights reserved. https://ljung.dev/.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CactusPlayerPawn.generated.h"

class UInputAction;
struct FInputActionValue;

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

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category="Input")
	FVector2D CameraSensitivity;

	UPROPERTY(EditAnywhere, Category="Input")
	FVector2D CameraPitchRange;

	ACactusPlayerPawn();

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void OnInput_Move(const FInputActionValue& Value);
	void OnInput_Look(const FInputActionValue& Value);
};
