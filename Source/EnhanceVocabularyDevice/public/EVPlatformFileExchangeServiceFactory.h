#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVPlatformFileExchangeServiceFactory.generated.h"

UCLASS()
class ENHANCEVOCABULARYDEVICE_API UEVPlatformFileExchangeServiceFactory : public UObject
{
    GENERATED_BODY()

public:
    static UObject* Create(UObject* Outer);
};