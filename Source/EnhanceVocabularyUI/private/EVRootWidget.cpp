// Fill out your copyright notice in the Description page of Project Settings.

#include "EVRootWidget.h"
#include "EnhanceVocabulary/EVGameInstance.h"

#include "Kismet/KismetSystemLibrary.h"

// Defining consts fpr the Color dynamic material instance params
const FName UEVRootWidget::SphereColorParam(TEXT("SphereColor"));
const FName UEVRootWidget::OpacityParam(TEXT("Opacity"));

void UEVRootWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    EVGameInstance = Cast<UEVGameInstance>(GetGameInstance());

    if (Image_ConnectionState)
    {
        ConnectionMID = Image_ConnectionState->GetDynamicMaterial();
        ConnectionMID->SetScalarParameterValue(OpacityParam, 1.0f);
    }

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
                if (bIsAppOnline)
                {
                    WidgetSwitcher_Main->SetActiveWidget(AddWord);
                    MenuSwitcherCount = 0;
                }
                else
                {
                    FEVErrorInfo EVErrorInfo;

                    EVErrorInfo.Source = EEVErrorSource::ConnectionModule;
                    EVErrorInfo.Type = EEVErrorType::ConnectionError;
                    EVErrorInfo.Message = FText::FromString(TEXT(
                        "Failed to connect to the WEB. You are Offline and this functionality may not be accessible."));

                    MenuSwitcherCount = 0;
                    OnRootWidgetError.Broadcast(EVErrorInfo);
                }
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
        if (bIsAppOnline)
        {
            WidgetSwitcher_Main->SetActiveWidget(AddWord);
            MenuSwitcherCount = 0;
        }
        else
        {
            FEVErrorInfo EVErrorInfo;

            EVErrorInfo.Source = EEVErrorSource::ConnectionModule;
            EVErrorInfo.Type = EEVErrorType::ConnectionError;
            EVErrorInfo.Message = FText::FromString(
                TEXT("Failed to connect to the WEB. You are Offline and this functionality may not be accessible."));

            MenuSwitcherCount = 0;
            OnRootWidgetError.Broadcast(EVErrorInfo);
        }
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
        bIsAppOnline = false;
        HandleConnectionImageColor(ConnectionMID, State);
        break;
    case EEVConnectionState::Connecting:
        bIsAppOnline = false;
        HandleConnectionImageColor(ConnectionMID, State);
        UE_LOG(LogTemp, Warning, TEXT("We are Connecting"));
        break;
    case EEVConnectionState::Online:
        bIsAppOnline = true;
        HandleConnectionImageColor(ConnectionMID, State);
        UE_LOG(LogTemp, Warning, TEXT("We are Online"));
        break;
    default:
        HandleConnectionImageColor(ConnectionMID, State);
        break;
    }
}

void UEVRootWidget::HandleConnectionImageColor(TObjectPtr<UMaterialInstanceDynamic> MaterialInstanceDynamic,
                                               EEVConnectionState ConnectionState)
{
    if (MaterialInstanceDynamic)
    {
        switch (ConnectionState)
        {
        case EEVConnectionState::Online:
            MaterialInstanceDynamic->SetVectorParameterValue(SphereColorParam,
                                                             FLinearColor(FColor(0x00, 0xBC, 0x00, 0xFF)));
            break;
        case EEVConnectionState::Connecting:
            MaterialInstanceDynamic->SetVectorParameterValue(SphereColorParam,
                                                             FLinearColor(FColor(0xE7, 0xE7, 0x00, 0xFF)));
            break;
        case EEVConnectionState::Offline:
            MaterialInstanceDynamic->SetVectorParameterValue(SphereColorParam,
                                                             FLinearColor(FColor(0xDA, 0x00, 0x00, 0xFF)));
            break;
        default:
            MaterialInstanceDynamic->SetVectorParameterValue(SphereColorParam,
                                                             FLinearColor(FColor(0xDA, 0x00, 0x00, 0xFF)));
            break;
        }
    }
    else
    {
        UE_LOG(LogTemp, Error,
               TEXT("TObjectPtr<UMaterialInstanceDynamic> MaterialInstanceDynamic is nullptr in WBP_RootWidget"));
    }
}
