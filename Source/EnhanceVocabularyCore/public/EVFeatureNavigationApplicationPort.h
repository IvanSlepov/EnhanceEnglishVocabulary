#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVApplicationFeatureTypes.h"
#include "EVFeatureNavigationApplicationPort.generated.h"

UINTERFACE(MinimalAPI)
class UEVFeatureNavigationApplicationPort : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVFeatureNavigationApplicationPort
{
    GENERATED_BODY()

public:
    virtual void ApplyFeatureNavigation(const FEVFeatureNavigationRequest& Request) = 0;
};
