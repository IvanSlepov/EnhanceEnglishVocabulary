// EVErrorDisplayWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVErrorTypes.h"
#include "EVErrorDisplayWidget.generated.h"

UINTERFACE(BlueprintType)
class ENHANCEVOCABULARYCORE_API UEVErrorDisplayWidget : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVErrorDisplayWidget
{
    GENERATED_BODY()

public:
    virtual void ShowError(const FText& ErrorText) = 0;
};