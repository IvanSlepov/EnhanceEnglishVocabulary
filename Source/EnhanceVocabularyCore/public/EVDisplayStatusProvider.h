// EVDisplayStatusProvider.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVRequestedActionTypes.h"
#include "EVDisplayStatusProvider.generated.h"

UINTERFACE(BlueprintType)
class ENHANCEVOCABULARYCORE_API UEVDisplayStatusProvider : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVDisplayStatusProvider
{
    GENERATED_BODY()

public:
    virtual void ShowStatus(const FEVRequestedActionInfo& RequestedActionInfo) = 0;
};