#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVVocabularyFilterTypes.h"
#include "EVVocabularyFilterApplicationPort.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVVocabularyFilterCriteriaChangeRequested, const FEVVocabularyQueryCriteria&);
DECLARE_MULTICAST_DELEGATE(FOnEVVocabularyFiltersRequested);

UINTERFACE(MinimalAPI)
class UEVVocabularyFilterApplicationPort : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVVocabularyFilterApplicationPort
{
    GENERATED_BODY()

public:
    virtual FOnEVVocabularyFiltersRequested& GetVocabularyFilterPresentationRequestedEvent() = 0;
    virtual FOnEVVocabularyFilterCriteriaChangeRequested& GetVocabularyFilterCriteriaChangeRequestedEvent() = 0;

    virtual void PresentVocabularyFilters(const FEVVocabularyQueryCriteria& Criteria) = 0;
    virtual void ApplyVocabularyFilterCriteria(const FEVVocabularyQueryCriteria& Criteria) = 0;
};
