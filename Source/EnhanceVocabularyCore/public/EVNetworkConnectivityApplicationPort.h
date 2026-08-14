#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVApplicationRequestTypes.h"
#include "EVNetworkConnectivityApplicationPort.generated.h"

UINTERFACE(MinimalAPI)
class UEVNetworkConnectivityApplicationPort : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVNetworkConnectivityApplicationPort
{
    GENERATED_BODY()

public:
    virtual void ApplyNetworkConnectivityState(EEVApplicationConnectivityState State) = 0;
};
