// EVWidgetCommonEvents.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVErrorTypes.h"
#include "EVRequestedActionTypes.h"
#include "EVWordEntryActionTypes.h"

#include "EVWidgetCommonEvents.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWidgetInteractionDisabled, bool, WidgetInteractionDisabled,
                                             const FString&, ObjectName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadingDataTriggerred, bool, bIsDataLoading);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionRequested, const FEVRequestedActionInfo&, RequestedActionInfo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWordEntryWidgetControlsActivated, const FEVWordEntryActionInfo&,
                                            WordEntryActionInfo);

UINTERFACE(BlueprintType)
class ENHANCEVOCABULARYCORE_API UEVWidgetCommonEvents : public UInterface
{
    GENERATED_BODY()
};

/*
 * Optional interface events return pointers instead of references.
 *
 * Not every widget implementing IEVWidgetCommonEvents exposes every event.
 * Returning nullptr explicitly signals that a particular event is not supported
 * by the implementing widget, allowing callers to safely check for its presence:
 *
 *     if (FOnLoadingDataTriggerred* Event = Widget->GetLoadingSpinnerEvent())
 *     {
 *         Event->AddDynamic(...);
 *     }
 *
 * Returning a reference would require every implementation to return a valid
 * delegate object, making the event effectively mandatory and preventing widgets
 * that do not participate in that event from using the interface cleanly.
 *
 * This keeps IEVWidgetCommonEvents as a common event interface for all widgets
 * while allowing each widget to expose only the delegates relevant to its role.
 */

class ENHANCEVOCABULARYCORE_API IEVWidgetCommonEvents
{
    GENERATED_BODY()

public:
    virtual FOnWidgetInteractionDisabled* GetWidgetInteractionDisabledEvent()
    {
        return nullptr;
    }

    virtual FOnLoadingDataTriggerred* GetLoadingSpinnerEvent()
    {
        return nullptr;
    }

    virtual FOnActionRequested* GetRequestedActionInfo()
    {
        return nullptr;
    }

    virtual FOnWordEntryWidgetControlsActivated* GetCurrentWordEntryWidgetActionInfo()
    {
        return nullptr;
    }

    virtual void HandleWordEntryChanged(const FEVWordEntryActionInfo& WordEntryActionInfo) {}
};