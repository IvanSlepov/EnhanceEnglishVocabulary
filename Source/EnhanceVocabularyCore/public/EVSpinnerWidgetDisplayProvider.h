// EVSpinnerWidgetDisplayProvider.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVSpinnerWidgetDisplayProvider.generated.h"

UINTERFACE(BlueprintType)
class ENHANCEVOCABULARYCORE_API UEVSpinnerWidgetDisplayProvider : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVSpinnerWidgetDisplayProvider
{
    GENERATED_BODY()

public:
    virtual void ShowSpinner() = 0;
};