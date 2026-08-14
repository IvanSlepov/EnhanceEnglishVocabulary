#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVFileExchangeTypes.h"
#include "EVFileExchangeApplicationPort.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVFileOperationRequested, const FEVFileOperationInfo&);

UINTERFACE(MinimalAPI)
class UEVFileExchangeApplicationPort : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVFileExchangeApplicationPort
{
    GENERATED_BODY()

public:
    virtual FOnEVFileOperationRequested& GetFileOperationRequestedEvent() = 0;
};
