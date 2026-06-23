// EVErrorProvider.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVErrorTypes.h"

#include "EVErrorProvider.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEVError, const FEVErrorInfo&, ErrorInfo);

UINTERFACE(BlueprintType)
class ENHANCEVOCABULARYCORE_API UEVErrorProvider : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVErrorProvider
{
    GENERATED_BODY()

public:
    virtual FOnEVError& GetOnErrorEvent() = 0;
};