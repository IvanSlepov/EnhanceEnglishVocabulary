#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVVocabularyTypes.h"
#include "EVWordEntryDisplayWidgetProvider.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UEVWordEntryDisplayWidgetProvider : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYCORE_API IEVWordEntryDisplayWidgetProvider
{
    GENERATED_BODY()

public:
    virtual void ShowWordEntry(const FVocabularyEntry& Entry) = 0;
    virtual void SetEditableFieldsReadOnly(bool bSetEnabled) = 0;

    virtual FSimpleMulticastDelegate& GetViewPressedDelegate() = 0;
    virtual FSimpleMulticastDelegate& GetEditPressedDelegate() = 0;
    virtual FSimpleMulticastDelegate& GetDeletePressedDelegate() = 0;
};