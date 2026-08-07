#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVVocabularyFilterTypes.h"
#include "EVVocabularyFilterWidgetProvider.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVocabularyFiltersApplied, const FEVVocabularyQueryCriteria&, Criteria);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVocabularyFilterWidgetCloseRequested);

UINTERFACE(BlueprintType)
class ENHANCEVOCABULARYCORE_API UEVVocabularyFilterWidgetProvider : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVVocabularyFilterWidgetProvider
{
    GENERATED_BODY()

public:
    virtual void SetInitialCriteria(const FEVVocabularyQueryCriteria& Criteria) = 0;
    virtual FOnVocabularyFiltersApplied& GetFiltersAppliedEvent() = 0;
    virtual FOnVocabularyFilterWidgetCloseRequested& GetCloseRequestedEvent() = 0;
};
