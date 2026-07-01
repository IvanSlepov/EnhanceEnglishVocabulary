// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVSelectWebProvidersWidget.h"
#include "EVWidgetControllable.h"
#include "EVWidgetCommonEvents.h"
#include "EVAppSettingsWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWebProvidersSelectionChangedSettingsWidget, EEVWebProvider,
                                             DefinitionUsageProvider, EEVWebProvider, TranslationProvider);
/**
 *
 */
UCLASS()
class ENHANCEVOCABULARYUI_API UEVAppSettingsWidget : public UUserWidget,
                                                     public IEVWidgetControllable,
                                                     public IEVWidgetCommonEvents
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UEVSelectWebProvidersWidget> WBP_SelectWebProviders;

    class UEVGameInstance* EVGameInstance;

    // Disable/Enable or Get controls status on demand
    virtual void SetControlsEnabled(bool bEnabled) override;
    virtual bool GetControlsEnabled() override;

    UPROPERTY(BlueprintAssignable)
    FOnWebProvidersSelectionChangedSettingsWidget OnWebProvidersSelectionChangedSettingsWidget;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    bool bAreInteractionElementsEnabled = true;

    UFUNCTION()
    void HandleDefinitionUsageProviderChanged(EEVWebProvider DefinitionUsageProvider,
                                              EEVWebProvider TranslationProvider);
};
