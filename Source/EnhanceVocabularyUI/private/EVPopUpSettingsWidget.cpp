// Fill out your copyright notice in the Description page of Project Settings.

#include "EVPopUpSettingsWidget.h"

void UEVPopUpSettingsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (ComboBoxString_PopUpIntervals)
    {
        ComboBoxString_PopUpIntervals->OnSelectionChanged.AddDynamic(this, &ThisClass::HandleOnPopUpIntervalSelected);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ComboBoxString_PopUpIntervals is nullptr in EVPopUpSettingsWidget.cpp"));
    }
}

void UEVPopUpSettingsWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UEVPopUpSettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    PopulatePopUpIntervals();
}

void UEVPopUpSettingsWidget::PopulatePopUpIntervals()
{
    if (!ComboBoxString_PopUpIntervals)
    {
        UE_LOG(LogTemp, Error, TEXT("ComboBoxString_PopUpIntervals is nullptr in EVPopUpSettingsWidget.cpp"));
        return;
    }

    FEVPopUpSettingsInfo EVPopUpSettingsInfo;

    for (auto Interval : EVPopUpSettingsInfo.GetAllIntervalDisplayNames())
    {
        FString IntervalName = Interval.ToString();
        ComboBoxString_PopUpIntervals->AddOption(IntervalName);
    }

    ComboBoxString_PopUpIntervals->SetSelectedOption(
        EVPopUpSettingsInfo.GetIntervalDisplayName(EEVPopUpIntervals::TurnedOff).ToString());
}

void UEVPopUpSettingsWidget::HandleOnPopUpIntervalSelected(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (bApplyingIntervalFromController)
    {
        return;
    }

    EEVPopUpIntervals SelectedInterval;

    if (!FEVPopUpSettingsInfo::TryGetIntervalFromDisplayName(SelectedItem, SelectedInterval))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to resolve pop-up interval from '%s'."), *SelectedItem);

        return;
    }

    FEVPopUpSettingsInfo PopUpSettings;
    PopUpSettings.PopUpIntervals = SelectedInterval;

    OnPopUpIntervalSelected.Broadcast(PopUpSettings);
}

void UEVPopUpSettingsWidget::SetSelectedInterval(const FEVPopUpSettingsInfo& PopUpSettingsInfo)
{
    if (!ComboBoxString_PopUpIntervals)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot set pop-up interval: ComboBoxString_PopUpIntervals is null."));

        return;
    }

    const FString IntervalDisplayName =
        PopUpSettingsInfo.GetIntervalDisplayName(PopUpSettingsInfo.PopUpIntervals).ToString();

    if (ComboBoxString_PopUpIntervals->FindOptionIndex(IntervalDisplayName) == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot set pop-up interval: option '%s' was not found."), *IntervalDisplayName);

        return;
    }

    bApplyingIntervalFromController = true;

    ComboBoxString_PopUpIntervals->SetSelectedOption(IntervalDisplayName);

    bApplyingIntervalFromController = false;
}