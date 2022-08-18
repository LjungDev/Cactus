// Copyright © Ljung 2022. All rights reserved. https://ljung.dev/.


#include "CactusPlayerController.h"

#include "EnhancedInputSubsystems.h"

void ACactusPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
	}
}
