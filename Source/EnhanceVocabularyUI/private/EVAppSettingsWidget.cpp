// Fill out your copyright notice in the Description page of Project Settings.

#include "EVAppSettingsWidget.h"

#include "EnhanceVocabulary/EVGameInstance.h"
#include "EVErrorTypes.h"
#include "EVRequestedActionTypes.h"

void UEVAppSettingsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (WBP_SelectWebProviders)
    {
        WBP_SelectWebProviders->OnWebProvidersSelectionChanged.AddDynamic(
            this, &ThisClass::HandleDefinitionUsageProviderChanged);
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
}