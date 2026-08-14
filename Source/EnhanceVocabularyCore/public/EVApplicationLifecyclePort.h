#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVApplicationLifecyclePort.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnEVApplicationExitRequested);

UINTERFACE(MinimalAPI)
class UEVApplicationLifecyclePort : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVApplicationLifecyclePort
{
    GENERATED_BODY()

public:
    virtual FOnEVApplicationExitRequested& GetApplicationExitRequestedEvent() = 0;
};
