#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVVocabularyTypes.h"
#include "EVEntryDetailsApplicationPort.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnEVEntryDetailsCloseRequested);
DECLARE_MULTICAST_DELEGATE(FOnEVEntryDetailsDeleteRequested);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVEntryDetailsSaveRequested, const FEVVocabularyRecord&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVEntryDetailsRequested, const FEVVocabularyRecord&);

UINTERFACE(MinimalAPI)
class UEVEntryDetailsApplicationPort : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVEntryDetailsApplicationPort
{
    GENERATED_BODY()

public:
    virtual FOnEVEntryDetailsRequested& GetEntryDetailsRequestedEvent() = 0;
    virtual FOnEVEntryDetailsCloseRequested& GetEntryDetailsCloseRequestedEvent() = 0;
    virtual FOnEVEntryDetailsDeleteRequested& GetEntryDetailsDeleteRequestedEvent() = 0;
    virtual FOnEVEntryDetailsSaveRequested& GetEntryDetailsSaveRequestedEvent() = 0;

    virtual void PresentEntryDetails(const FEVVocabularyRecord& Record) = 0;
    virtual void DismissEntryDetails() = 0;
};
