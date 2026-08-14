// Fill out your copyright notice in the Description page of Project Settings.

#include "EVAppSettingsWidget.h"

#include "EVErrorTypes.h"
#include "EVRequestedActionTypes.h"
#include "EVDBLanguageContextAndTranslationsWidget.h"

void UEVAppSettingsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (WBP_SelectWebProviders)
    {
        WBP_SelectWebProviders->OnWebProvidersSelectionChanged.AddDynamic(
            this, &ThisClass::HandleDefinitionUsageProviderChanged);
    }

    if (WBP_DBLanguageContextAndTranslations)
    {
        WBP_DBLanguageContextAndTranslations->OnPreferencesChanged.AddUniqueDynamic(
            this, &ThisClass::HandleVocabularyLanguagePreferencesChanged);
    }
}

void UEVAppSettingsWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UEVAppSettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UEVAppSettingsWidget::SetControlsEnabled(bool bEnabled)
{
    bAreInteractionElementsEnabled = bEnabled;
}

bool UEVAppSettingsWidget::GetControlsEnabled()
{
    return bAreInteractionElementsEnabled;
}

void UEVAppSettingsWidget::HandleDefinitionUsageProviderChanged(EEVWebProvider DefinitionUsageProvider,
                                                                EEVWebProvider TranslationProvider)
{
    OnWebProvidersSelectionChangedSettingsWidget.Broadcast(DefinitionUsageProvider, TranslationProvider);
    OnWebProviderSelectionChanged.Broadcast(DefinitionUsageProvider, TranslationProvider);
}

void UEVAppSettingsWidget::ApplyVocabularyLanguagePreferences(const FEVVocabularyLanguagePreferences& Preferences)
{
    if (WBP_DBLanguageContextAndTranslations)
    {
        WBP_DBLanguageContextAndTranslations->SetInitialPreferences(Preferences);
    }
}

void UEVAppSettingsWidget::HandleVocabularyLanguagePreferencesChanged(
    const FEVVocabularyLanguagePreferences& Preferences)
{
    OnVocabularyLanguagePreferencesChangedSettingsWidget.Broadcast(Preferences);

    FEVVocabularyPreferencesChangeRequest Request;
    Request.RequestId = FGuid::NewGuid();
    Request.Preferences = Preferences;
    OnVocabularyPreferencesChangeRequested.Broadcast(Request);
}

void UEVAppSettingsWidget::ApplyVocabularyPreferencesState(const FEVVocabularyPreferencesState& State)
{
    if (State.Result == EEVApplicationOperationResult::Succeeded)
    {
        ApplyVocabularyLanguagePreferences(State.Preferences);
    }
}

void UEVAppSettingsWidget::ApplyFeatureActivationState(const bool bIsActive)
{
    if (!bIsActive && WBP_DBLanguageContextAndTranslations)
    {
        WBP_DBLanguageContextAndTranslations->ResetPendingChanges();
    }
}
