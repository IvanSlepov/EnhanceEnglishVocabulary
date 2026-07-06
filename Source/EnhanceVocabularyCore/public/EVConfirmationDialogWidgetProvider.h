#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVConfirmationDialogActionTypes.h"
#include "EVConfirmationDialogWidgetProvider.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnConfirmationDialogButtonPressed, bool);

UINTERFACE(MinimalAPI, Blueprintable)
class UEVConfirmationDialogWidgetProvider : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVConfirmationDialogWidgetProvider
{
    GENERATED_BODY()

public:
    virtual void SetConfirmationDialogInfo(const FEVConfirmationDialogInfo& EVConfirmationDialogInfo) = 0;

    virtual FOnConfirmationDialogButtonPressed& GetDialogButtonPressedDelegate() = 0;
};