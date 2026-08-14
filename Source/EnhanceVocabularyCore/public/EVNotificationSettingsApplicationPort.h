#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVPopUpSettingsTypes.h"
#include "EVNotificationSettingsApplicationPort.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVNotificationSettingsChangeRequested, const FEVPopUpSettingsInfo&);

UINTERFACE(MinimalAPI)
class UEVNotificationSettingsApplicationPort : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVNotificationSettingsApplicationPort
{
    GENERATED_BODY()

public:
    virtual FOnEVNotificationSettingsChangeRequested& GetNotificationSettingsChangeRequestedEvent() = 0;
    virtual void ApplyNotificationSettingsState(const FEVPopUpSettingsInfo& Settings) = 0;
};
