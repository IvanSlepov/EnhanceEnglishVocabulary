// EVWidgetErrorProvider.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVErrorTypes.h"
#include "EVWidgetErrorProvider.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEVWidgetError, const FEVErrorInfo&, ErrorInfo);

UINTERFACE(BlueprintType)
class ENHANCEVOCABULARYCORE_API UEVWidgetErrorProvider : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVWidgetErrorProvider
{
    GENERATED_BODY()

public:
    virtual FOnEVWidgetError& GetOnWidgetErrorEvent() = 0;
};