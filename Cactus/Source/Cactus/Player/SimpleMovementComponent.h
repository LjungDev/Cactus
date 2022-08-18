// Copyright © Ljung 2022. All rights reserved. https://ljung.dev/.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SimpleMovementComponent.generated.h"

UCLASS()
class CACTUS_API USimpleMovementComponent final : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Movement")
	float MoveSpeed;

	USimpleMovementComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
