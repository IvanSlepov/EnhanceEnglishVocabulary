// Fill out your copyright notice in the Description page of Project Settings.

#include "EVPopUpSettingsWidget.h"

void UEVPopUpSettingsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (ComboBoxString_NotificationIntervals)
    {
        ComboBoxString_NotificationIntervals->OnSelectionChanged.AddDynamic(
            this, &ThisClass::HandleOnNotificationsIntervalSelected);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ComboBoxString_NotificationIntervals is nullptr in EVPopUpSettingsWidget.cpp"));
    }

    if (ComboBoxString_NotificationModes)
    {
        ComboBoxString_NotificationModes->OnSelectionChanged.AddDynamic(this,
                                                                        &ThisClass::HandleOnNotificationsModeSelected);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ComboBoxString_NotificationModes is nullptr in EVPopUpSettingsWidget.cpp"));
    }
}

void UEVPopUpSettingsWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UEVPopUpSettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    bApplyingSettingsFromController = true;

    PopulateNotificationIntervals();
    PopulateNotificationModes();

    SetSelectedSettings(EVNotificationSettingsInfo);

    bApplyingSettingsFromController = false;
}

void UEVPopUpSettingsWidget::PopulateNotificationIntervals()
{
    if (!ComboBoxString_NotificationIntervals)
    {
        UE_LOG(LogTemp, Error, TEXT("ComboBoxString_NotificationIntervals is nullptr in EVPopUpSettingsWidget.cpp"));
        return;
    }

    ComboBoxString_NotificationIntervals->ClearOptions();

    for (const FText& Interval : FEVPopUpSettingsInfo::GetAllIntervalDisplayNames())
    {
        ComboBoxString_NotificationIntervals->AddOption(Interval.ToString());
    }
}

void UEVPopUpSettingsWidget::PopulateNotificationModes()
{
    if (!ComboBoxString_NotificationModes)
    {
        UE_LOG(LogTemp, Error, TEXT("ComboBoxString_NotificationModes is nullptr in EVPopUpSettingsWidget.cpp"));
        return;
    }

    ComboBoxString_NotificationModes->ClearOptions();

    for (const FText& Mode : FEVPopUpSettingsInfo::GetAllNotificationModesDisplayNames())
    {
        ComboBoxString_NotificationModes->AddOption(Mode.ToString());
    }
}

void UEVPopUpSettingsWidget::BroadcastCurrentSettings()
{
    if (!ComboBoxString_NotificationIntervals || !ComboBoxString_NotificationModes)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot broadcast notification settings: one or more combo boxes are null."));

        return;
    }

    const FString SelectedIntervalName = ComboBoxString_NotificationIntervals->GetSelectedOption();

    const FString SelectedModeName = ComboBoxString_NotificationModes->GetSelectedOption();

    EEVPopUpIntervals SelectedInterval;
    EEVNotificationMode SelectedMode;

    if (!FEVPopUpSettingsInfo::TryGetIntervalFromDisplayName(SelectedIntervalName, SelectedInterval))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to resolve notification interval from '%s'."), *SelectedIntervalName);

        return;
    }

    if (!FEVPopUpSettingsInfo::TryGetNotificationModeFromDisplayName(SelectedModeName, SelectedMode))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to resolve notification mode from '%s'."), *SelectedModeName);

        return;
    }

    EVNotificationSettingsInfo.PopUpIntervals = SelectedInterval;
    EVNotificationSettingsInfo.NotificationMode = SelectedMode;

    OnNotificationSettingsChanged.Broadcast(EVNotificationSettingsInfo);
    OnFeatureNotificationSettingsChanged.Broadcast(EVNotificationSettingsInfo);
}

void UEVPopUpSettingsWidget::HandleOnNotificationsIntervalSelected(FString SelectedItem,
                                                                   ESelectInfo::Type SelectionType)
{
    if (bApplyingSettingsFromController)
    {
        return;
    }

    BroadcastCurrentSettings();
}

void UEVPopUpSettingsWidget::HandleOnNotificationsModeSelected(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (bApplyingSettingsFromController)
    {
        return;
    }

    BroadcastCurrentSettings();
}

void UEVPopUpSettingsWidget::SetSelectedSettings(const FEVPopUpSettingsInfo& PopUpSettingsInfo)
{
    if (!ComboBoxString_NotificationIntervals || !ComboBoxString_NotificationModes)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot set notification settings: one or more combo boxes are null."));
        return;
    }

    const FString IntervalDisplayName =
        FEVPopUpSettingsInfo::GetIntervalDisplayName(PopUpSettingsInfo.PopUpIntervals).ToString();
    const FString ModeDisplayName =
        FEVPopUpSettingsInfo::GetNotificationModesDisplayName(PopUpSettingsInfo.NotificationMode).ToString();

    if (ComboBoxString_NotificationIntervals->FindOptionIndex(IntervalDisplayName) == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot set notification interval: option '%s' was not found."),
               *IntervalDisplayName);
        return;
    }

    if (ComboBoxString_NotificationModes->FindOptionIndex(ModeDisplayName) == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot set notification mode: option '%s' was not found."), *ModeDisplayName);
        return;
    }

    bApplyingSettingsFromController = true;

    ComboBoxString_NotificationIntervals->SetSelectedOption(IntervalDisplayName);
    ComboBoxString_NotificationModes->SetSelectedOption(ModeDisplayName);
    EVNotificationSettingsInfo = PopUpSettingsInfo;

    bApplyingSettingsFromController = false;
}
