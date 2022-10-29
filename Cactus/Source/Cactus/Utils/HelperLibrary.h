// Copyright © Ljung 2022. All rights reserved. https://ljung.dev/.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HelperLibrary.generated.h"

UCLASS()
class CACTUS_API UHelperLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	template <typename T>
	static FString EnumToString(const FString& EnumName, const T Value)
	{
		const UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
		return Enum
			       ? Enum->GetDisplayNameTextByIndex(static_cast<uint8>(Value)).ToString()
			       : TEXT("<invalid>");
	}

	FORCEINLINE static FString PrettyBool(const bool Value)
	{
		if (Value) return TEXT("True");
		return TEXT("False");
	}
};
