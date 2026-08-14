#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVErrorTypes.h"
#include "EVRequestedActionTypes.h"
#include "EVGlobalPresentationApplicationPort.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVGlobalErrorRequested, const FEVErrorInfo&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVGlobalLoadingStateChanged, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVGlobalStatusRequested, const FEVRequestedActionInfo&);

UINTERFACE(MinimalAPI)
class UEVGlobalPresentationApplicationPort : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVGlobalPresentationApplicationPort
{
    GENERATED_BODY()

public:
    virtual FOnEVGlobalErrorRequested& GetGlobalErrorRequestedEvent() = 0;
    virtual FOnEVGlobalLoadingStateChanged& GetGlobalLoadingStateChangedEvent() = 0;
    virtual FOnEVGlobalStatusRequested& GetGlobalStatusRequestedEvent() = 0;
};
