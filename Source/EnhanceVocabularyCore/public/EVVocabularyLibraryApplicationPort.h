#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVApplicationRequestTypes.h"
#include "EVVocabularyLibraryApplicationPort.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVVocabularyRecordRequested, const FEVVocabularyRecordRequest&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVVocabularyQueryRequested, const FEVVocabularyQueryRequest&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVVocabularyMutationRequested, const FEVVocabularyMutationRequest&);

UINTERFACE(MinimalAPI)
class UEVVocabularyLibraryApplicationPort : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVVocabularyLibraryApplicationPort
{
    GENERATED_BODY()

public:
    virtual FOnEVVocabularyRecordRequested& GetVocabularyRecordRequestedEvent() = 0;
    virtual FOnEVVocabularyQueryRequested& GetVocabularyQueryRequestedEvent() = 0;
    virtual FOnEVVocabularyMutationRequested& GetVocabularyMutationRequestedEvent() = 0;

    virtual void ApplyVocabularyRecordOutcome(const FEVVocabularyRecordOutcome& Outcome) = 0;
    virtual void ApplyVocabularyQueryOutcome(const FEVVocabularyQueryOutcome& Outcome) = 0;
    virtual void ApplyVocabularyMutationOutcome(const FEVVocabularyMutationOutcome& Outcome) = 0;
    virtual void ApplyVocabularyChanged(const FEVVocabularyChangeInfo& ChangeInfo) = 0;
};
