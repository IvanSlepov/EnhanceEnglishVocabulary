#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVVocabularyInteractionTypes.h"
#include "EVVocabularyValueApplicationPort.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVVocabularyValueActionRequested, const FEVVocabularyValueActionRequest&);

UINTERFACE(MinimalAPI)
class UEVVocabularyValueApplicationPort : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVVocabularyValueApplicationPort
{
    GENERATED_BODY()

public:
    virtual FOnEVVocabularyValueActionRequested& GetVocabularyValueApplicationRequestedEvent() = 0;
};
