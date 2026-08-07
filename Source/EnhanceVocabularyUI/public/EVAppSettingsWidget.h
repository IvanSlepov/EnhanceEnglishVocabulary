// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVSelectWebProvidersWidget.h"
#include "EVWidgetControllable.h"
#include "EVWidgetCommonEvents.h"
#include "EVVocabularyLanguageTypes.h"
#include "EVAppSettingsWidget.generated.h"

class UEVDBLanguageContextAndTranslationsWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWebProvidersSelectionChangedSettingsWidget, EEVWebProvider,
                                             DefinitionUsageProvider, EEVWebProvider, TranslationProvider);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVocabularyLanguagePreferencesChangedSettingsWidget,
                                            const FEVVocabularyLanguagePreferences&, Preferences);
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UEVDBLanguageContextAndTranslationsWidget> WBP_DBLanguageContextAndTranslations;

    class UEVGameInstance* EVGameInstance;

    // Disable/Enable or Get controls status on demand
    virtual void SetControlsEnabled(bool bEnabled) override;
    virtual bool GetControlsEnabled() override;

    UPROPERTY(BlueprintAssignable)
    FOnWebProvidersSelectionChangedSettingsWidget OnWebProvidersSelectionChangedSettingsWidget;

    UPROPERTY(BlueprintAssignable)
    FOnVocabularyLanguagePreferencesChangedSettingsWidget OnVocabularyLanguagePreferencesChangedSettingsWidget;

    void ApplyVocabularyLanguagePreferences(const FEVVocabularyLanguagePreferences& Preferences);

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    bool bAreInteractionElementsEnabled = true;

    UFUNCTION()
    void HandleDefinitionUsageProviderChanged(EEVWebProvider DefinitionUsageProvider,
                                              EEVWebProvider TranslationProvider);

    UFUNCTION()
    void HandleVocabularyLanguagePreferencesChanged(const FEVVocabularyLanguagePreferences& Preferences);
};
