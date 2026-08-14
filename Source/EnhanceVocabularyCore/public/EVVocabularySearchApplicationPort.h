#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVApplicationRequestTypes.h"
#include "EVVocabularySearchApplicationPort.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVVocabularySearchRequested, const FEVVocabularySearchRequest&);

UINTERFACE(MinimalAPI)
class UEVVocabularySearchApplicationPort : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVVocabularySearchApplicationPort
{
    GENERATED_BODY()

public:
    virtual FOnEVVocabularySearchRequested& GetVocabularySearchRequestedEvent() = 0;
    virtual void ApplyVocabularySearchOutcome(const FEVVocabularySearchOutcome& Outcome) = 0;
};
