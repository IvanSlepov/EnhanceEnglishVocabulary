// EVWidgetCommonEvents.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVErrorTypes.h"

#include "EVWidgetCommonEvents.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWidgetInteractionDisabled, bool, WidgetInteractionDisabled,
                                             const FString&, ObjectName);

UINTERFACE(BlueprintType)
class ENHANCEVOCABULARYCORE_API UEVWidgetCommonEvents : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVWidgetCommonEvents
{
    GENERATED_BODY()

public:
    virtual FOnWidgetInteractionDisabled& GetWidgetInteractionDisabledEvent() = 0;
};