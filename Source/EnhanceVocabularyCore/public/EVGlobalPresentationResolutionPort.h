#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVErrorTypes.h"
#include "EVGlobalPresentationResolutionPort.generated.h"

UINTERFACE(MinimalAPI)
class UEVGlobalPresentationResolutionPort : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVGlobalPresentationResolutionPort
{
    GENERATED_BODY()

public:
    virtual void ApplyGlobalErrorResolution(const FEVErrorInfo& ErrorInfo) = 0;
};
