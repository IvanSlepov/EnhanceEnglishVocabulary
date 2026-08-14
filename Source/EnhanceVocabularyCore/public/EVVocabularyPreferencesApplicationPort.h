#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVApplicationRequestTypes.h"
#include "EVVocabularyPreferencesApplicationPort.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVVocabularyPreferencesChangeRequested,
                                    const FEVVocabularyPreferencesChangeRequest&);

UINTERFACE(MinimalAPI)
class UEVVocabularyPreferencesApplicationPort : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVVocabularyPreferencesApplicationPort
{
    GENERATED_BODY()

public:
    virtual FOnEVVocabularyPreferencesChangeRequested& GetVocabularyPreferencesChangeRequestedEvent() = 0;
    virtual void ApplyVocabularyPreferencesState(const FEVVocabularyPreferencesState& State) = 0;
};
