// Fill out your copyright notice in the Description page of Project Settings.

#include "EVMainMenuWidget.h"

void UEVMainMenuWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_AddWord && Button_ImportExport && Button_PopupSettings && Button_Quit && Button_ReviewWords)
    {
        Button_AddWord->OnPressed.AddDynamic(this, &ThisClass::HandleAddWordButtonPressed);
        Button_ReviewWords->OnPressed.AddDynamic(this, &ThisClass::HandleReviewWordsButtonPressed);
        Button_ImportExport->OnPressed.AddDynamic(this, &ThisClass::HandleImportExportButtonPressed);
        Button_PopupSettings->OnPressed.AddDynamic(this, &ThisClass::HandlePopupSettingsButtonPressed);
        Button_Quit->OnPressed.AddDynamic(this, &ThisClass::HandleQuitButtonPressed);
    }
}

void UEVMainMenuWidget::NativePreConstruct() {}

void UEVMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    bIsAddWordActivated = false;
    bIsReviewWordsActivated = false;
    bIsPopupSettingsActivated = false;
    bIsImportExportActivated = false;
}

void UEVMainMenuWidget::HandleAddWordButtonPressed()
{
    if (!bIsAddWordActivated)
    {
        bIsAddWordActivated = true;

        bIsReviewWordsActivated = false;
        bIsPopupSettingsActivated = false;
        bIsImportExportActivated = false;

        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated);
    }
    else
    {
        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated);
    }
}

void UEVMainMenuWidget::HandleReviewWordsButtonPressed()
{
    if (!bIsReviewWordsActivated)
    {
        bIsReviewWordsActivated = true;

        bIsAddWordActivated = false;
        bIsPopupSettingsActivated = false;
        bIsImportExportActivated = false;

        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated);
    }
    else
    {
        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated);
    }
}

void UEVMainMenuWidget::HandlePopupSettingsButtonPressed()
{
    if (!bIsPopupSettingsActivated)
    {
        bIsPopupSettingsActivated = true;

        bIsAddWordActivated = false;
        bIsReviewWordsActivated = false;
        bIsImportExportActivated = false;

        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated);
    }
    else
    {
        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated);
    }
}

void UEVMainMenuWidget::HandleImportExportButtonPressed()
{
    if (!bIsImportExportActivated)
    {
        bIsImportExportActivated = true;

        bIsPopupSettingsActivated = false;
        bIsAddWordActivated = false;
        bIsReviewWordsActivated = false;

        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated);
    }
    else
    {
        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated);
    }
}

void UEVMainMenuWidget::HandleQuitButtonPressed()
{
    OnQuitButtonPressed.Broadcast();
}
