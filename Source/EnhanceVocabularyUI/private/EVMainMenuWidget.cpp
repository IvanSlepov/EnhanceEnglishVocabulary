// Fill out your copyright notice in the Description page of Project Settings.

#include "EVMainMenuWidget.h"

void UEVMainMenuWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_AddWord)
    {
        Button_AddWord->OnPressed.AddDynamic(this, &ThisClass::HandleAddWordButtonPressed);
    }
    if (Button_ReviewWords)
    {
        Button_ReviewWords->OnPressed.AddDynamic(this, &ThisClass::HandleReviewWordsButtonPressed);
    }
    if (Button_ImportExport)
    {
        Button_ImportExport->OnPressed.AddDynamic(this, &ThisClass::HandleImportExportButtonPressed);
    }
    if (Button_PopupSettings)
    {
        Button_PopupSettings->OnPressed.AddDynamic(this, &ThisClass::HandlePopupSettingsButtonPressed);
    }
    if (Button_AppSettings)
    {
        Button_AppSettings->OnPressed.AddDynamic(this, &ThisClass::HandleAppSettingsButtonPressed);
    }
    if (Button_Quit)
    {
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
    bIsAppSettingsActivated = false;
}

void UEVMainMenuWidget::HandleAddWordButtonPressed()
{
    OnFeatureNavigationRequested.Broadcast(EVApplicationFeature::AddWord);

    if (!bIsAddWordActivated)
    {
        bIsAddWordActivated = true;

        bIsReviewWordsActivated = false;
        bIsPopupSettingsActivated = false;
        bIsImportExportActivated = false;
        bIsAppSettingsActivated = false;

        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated, bIsAppSettingsActivated);
    }
    else
    {
        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated, bIsAppSettingsActivated);
    }
}

void UEVMainMenuWidget::HandleReviewWordsButtonPressed()
{
    OnFeatureNavigationRequested.Broadcast(EVApplicationFeature::ReviewWords);

    if (!bIsReviewWordsActivated)
    {
        bIsReviewWordsActivated = true;

        bIsAddWordActivated = false;
        bIsPopupSettingsActivated = false;
        bIsImportExportActivated = false;
        bIsAppSettingsActivated = false;

        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated, bIsAppSettingsActivated);
    }
    else
    {
        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated, bIsAppSettingsActivated);
    }
}

void UEVMainMenuWidget::HandlePopupSettingsButtonPressed()
{
    OnFeatureNavigationRequested.Broadcast(EVApplicationFeature::NotificationSettings);

    if (!bIsPopupSettingsActivated)
    {
        bIsPopupSettingsActivated = true;

        bIsAddWordActivated = false;
        bIsReviewWordsActivated = false;
        bIsImportExportActivated = false;
        bIsAppSettingsActivated = false;

        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated, bIsAppSettingsActivated);
    }
    else
    {
        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated, bIsAppSettingsActivated);
    }
}

void UEVMainMenuWidget::HandleImportExportButtonPressed()
{
    OnFeatureNavigationRequested.Broadcast(EVApplicationFeature::ImportExport);

    if (!bIsImportExportActivated)
    {
        bIsImportExportActivated = true;

        bIsPopupSettingsActivated = false;
        bIsAddWordActivated = false;
        bIsReviewWordsActivated = false;
        bIsAppSettingsActivated = false;

        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated, bIsAppSettingsActivated);
    }
    else
    {
        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated, bIsAppSettingsActivated);
    }
}

void UEVMainMenuWidget::HandleAppSettingsButtonPressed()
{
    OnFeatureNavigationRequested.Broadcast(EVApplicationFeature::ApplicationSettings);

    if (!bIsAppSettingsActivated)
    {
        bIsAppSettingsActivated = true;

        bIsImportExportActivated = false;
        bIsPopupSettingsActivated = false;
        bIsAddWordActivated = false;
        bIsReviewWordsActivated = false;

        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated, bIsAppSettingsActivated);
    }
    else
    {
        OnMenuButtonsPressed.Broadcast(bIsAddWordActivated, bIsReviewWordsActivated, bIsPopupSettingsActivated,
                                       bIsImportExportActivated, bIsAppSettingsActivated);
    }
}

void UEVMainMenuWidget::HandleQuitButtonPressed()
{
    OnApplicationExitRequested.Broadcast();
    OnQuitButtonPressed.Broadcast();
}

void UEVMainMenuWidget::SetFeatureAvailable(const FName FeatureId, const bool bAvailable)
{
    UButton* TargetButton = nullptr;

    if (FeatureId == EVApplicationFeature::AddWord)
    {
        TargetButton = Button_AddWord;
    }
    else if (FeatureId == EVApplicationFeature::ReviewWords)
    {
        TargetButton = Button_ReviewWords;
    }
    else if (FeatureId == EVApplicationFeature::NotificationSettings)
    {
        TargetButton = Button_PopupSettings;
    }
    else if (FeatureId == EVApplicationFeature::ImportExport)
    {
        TargetButton = Button_ImportExport;
    }
    else if (FeatureId == EVApplicationFeature::ApplicationSettings)
    {
        TargetButton = Button_AppSettings;
    }

    if (TargetButton)
    {
        TargetButton->SetIsEnabled(bAvailable);
    }
}
