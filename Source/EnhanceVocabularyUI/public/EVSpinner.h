// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVWidgetCommonEvents.h"
#include "EVSpinner.generated.h"

/**
 *
 */
UCLASS()
class ENHANCEVOCABULARYUI_API UEVSpinner : public UUserWidget, public IEVWidgetCommonEvents
{
    GENERATED_BODY()

public:
    void HandleRemoveFromParent();

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
};
