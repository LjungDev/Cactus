// Copyright © Ljung 2022. All rights reserved. https://ljung.dev/.

#pragma once

#include "CoreMinimal.h"
#include "CactusPlayerController.generated.h"

UCLASS()
class CACTUS_API ACactusPlayerController final : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	class UInputMappingContext* DefaultInputMappingContext;

protected:
	virtual void BeginPlay() override;
};
