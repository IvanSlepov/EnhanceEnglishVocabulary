// Fill out your copyright notice in the Description page of Project Settings.

#include "EVRootWidget.h"
#include "EnhanceVocabulary/EVGameInstance.h"

#include "Kismet/KismetSystemLibrary.h"

void UEVRootWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    EVGameInstance = Cast<UEVGameInstance>(GetGameInstance());

    if (EVGameInstance)
    {
        EVGameInstance->OnConnectionStateChanged.AddDynamic(this, &ThisClass::HandleOnConnectionStateChanged);
        HandleOnConnectionStateChanged(EVGameInstance->GetConnectionState());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("EVGameInstance in EVRootWidget.cpp is nullptr"));
    }

    bIsAddWordActivated_internal = false;
    bIsReviewWordsActivated_internal = false;
    bIsPopupSettingsActivated_internal = false;
    bIsImportExportActivated_internal = false;

    if (AddWord)
    {
        AddWord->OnError.AddDynamic(this, &ThisClass::HandleOnAnyWidgedErrorDetected);
    }

    if (Button_Menu)
    {
        Button_Menu->OnPressed.AddDynamic(this, &ThisClass::ButtonMenuPressed);
    }

    if (MainMenu)
    {
        MainMenu->OnMenuButtonsPressed.AddDynamic(this, &ThisClass::HandleMenuButtonsPressed);
        MainMenu->OnQuitButtonPressed.AddDynamic(this, &ThisClass::HandleQuitButtonPressed);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to instantiate WBP_MainMenu"));
    }
}

void UEVRootWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UEVRootWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (WidgetSwitcher_Main && NoMenu)
    {
        WidgetSwitcher_Main->SetActiveWidget(NoMenu);
        bIsAnyMenuActivated = false;
        MenuSwitcherCount = 0;
    }
}

void UEVRootWidget::ButtonMenuPressed()
{
    if (MenuSwitcherCount == 0)
    {
        WidgetSwitcher_Main->SetActiveWidget(MainMenu);
        MenuSwitcherCount = 1;
    }

    else
    {
        if (bIsAnyMenuActivated)
        {
            if (bIsAddWordActivated_internal)
            {
                WidgetSwitcher_Main->SetActiveWidget(AddWord);
                MenuSwitcherCount = 0;
            }

            else if (bIsReviewWordsActivated_internal)
            {
                WidgetSwitcher_Main->SetActiveWidget(ReviewWords);
                MenuSwitcherCount = 0;
            }
        }

        else
        {
            WidgetSwitcher_Main->SetActiveWidget(NoMenu);
            MenuSwitcherCount = 0;
        }
    }
}

void UEVRootWidget::HandleMenuButtonsPressed(bool bIsAddWordActivated, bool bIsReviewWordsActivated,
                                             bool bIsPopupSettingsActivated, bool bIsImportExportActivated)
{
    bIsAddWordActivated_internal = bIsAddWordActivated;
    bIsReviewWordsActivated_internal = bIsReviewWordsActivated;
    bIsPopupSettingsActivated_internal = bIsPopupSettingsActivated;
    bIsImportExportActivated_internal = bIsImportExportActivated;

    bIsAnyMenuActivated = true;

    if (bIsAddWordActivated)
    {
        WidgetSwitcher_Main->SetActiveWidget(AddWord);
        MenuSwitcherCount = 0;
    }

    else if (bIsReviewWordsActivated)
    {
        WidgetSwitcher_Main->SetActiveWidget(ReviewWords);
        ReviewWords->DisplayWords();
        MenuSwitcherCount = 0;
    }

    else if (bIsPopupSettingsActivated)
    {
    }

    else if (bIsImportExportActivated)
    {
    }
}

void UEVRootWidget::HandleQuitButtonPressed()
{
    UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UEVRootWidget::HandleOnAnyWidgedErrorDetected(const FEVErrorInfo& WidgetErrorInfo)
{
    OnRootWidgetError.Broadcast(WidgetErrorInfo);
}

void UEVRootWidget::HandleOnConnectionStateChanged(EEVConnectionState State)
{
    switch (State)
    {
    case EEVConnectionState::Offline:
        UE_LOG(LogTemp, Warning, TEXT("We are Offline"));
        break;
    case EEVConnectionState::Connecting:
        UE_LOG(LogTemp, Warning, TEXT("We are Connecting"));
        break;
    case EEVConnectionState::Online:
        UE_LOG(LogTemp, Warning, TEXT("We are Online"));
        break;
    default:
        break;
    }
}
