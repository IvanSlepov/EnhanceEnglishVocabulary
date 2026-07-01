// Fill out your copyright notice in the Description page of Project Settings.

#include "EVAppPlayerController.h"

#include "EVErrorProvider.h"
#include "EVGameInstance.h"
#include "EVErrorDisplayWidget.h"
#include "EVDisplayStatusProvider.h"
#include "EVWidgetCommonEvents.h"

AEVAppPlayerController::AEVAppPlayerController()
{
    bShowMouseCursor = false;
    bEnableClickEvents = true;
    bEnableTouchEvents = true;
    bEnableMouseOverEvents = false;
    bEnableTouchOverEvents = false;
}

void AEVAppPlayerController::BeginPlay()
{
    Super::BeginPlay();

    InitEVAppPlayerController();
}

void AEVAppPlayerController::InitEVAppPlayerController()
{
    ActivateTouchInterface(nullptr);

    FInputModeUIOnly InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);

    if (RootWidgetClass)
    {
        RootWidgetInstance = CreateWidget<UUserWidget>(this, RootWidgetClass);

        if (RootWidgetInstance)
        {
            RootWidgetInstance->AddToViewport();

            if (IEVErrorProvider* ErrorProvider = Cast<IEVErrorProvider>(RootWidgetInstance))
            {
                ErrorProvider->GetOnErrorEvent().AddDynamic(this, &ThisClass::HandleWidgetErrors);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to create instance of IEVErrorProvider in EVAppPlayerController"));
            }

            if (IEVWidgetCommonEvents* WidgetCommonEvents = Cast<IEVWidgetCommonEvents>(RootWidgetInstance))
            {
                if (FOnLoadingDataTriggerred* LoadingDataTriggerredEvent = WidgetCommonEvents->GetLoadingSpinnerEvent())
                {
                    LoadingDataTriggerredEvent->AddDynamic(this, &ThisClass::HandleLoadingSpinner);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("FOnLoadingDataTriggerred in EVAppPlayerController.cpp is nullptr"));
                }

                if (FOnActionRequested* ActionRequested = WidgetCommonEvents->GetRequestedActionInfo())
                {
                    ActionRequested->AddDynamic(this, &ThisClass::HandleActionStatusWidget);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("FOnActionRequested in EVAppPlayerController.cpp is nullptr"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Error,
                       TEXT("Failed to create instance of IEVWidgetCommonEvents in EVAppPlayerController"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create instance of RootWidgetClass in EVAppPlayerController"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("The RootWidgetClass was not provided to EVAppPlayerController"));
    }
}

void AEVAppPlayerController::HandleWidgetErrors(const FEVErrorInfo& WidgetErrorInfo)
{
    if (ErrorWidgetClass)
    {
        ErrorWidgetInstance = CreateWidget<UUserWidget>(this, ErrorWidgetClass);

        if (ErrorWidgetInstance)
        {
            if (IEVErrorDisplayWidget* ErrorDisplay = Cast<IEVErrorDisplayWidget>(ErrorWidgetInstance))
            {
                ErrorDisplay->ShowError(WidgetErrorInfo.Message);
            }

            ErrorWidgetInstance->AddToViewport(9999);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create instance of ErrorWidgetClass in EVAppPlayerController"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("The ErrorWidgetClass was not provided to EVAppPlayerController"));
    }
}

void AEVAppPlayerController::HandleLoadingSpinner(bool bDisplayLoadingSpinner)
{
    if (!LoadingSpinnerClass)
    {
        UE_LOG(LogTemp, Error, TEXT("The LoadingSpinnerClass was not provided to EVAppPlayerController"));
        return;
    }

    if (bDisplayLoadingSpinner)
    {
        if (!LoadingSpinnerInstance)
        {
            LoadingSpinnerInstance = CreateWidget<UUserWidget>(this, LoadingSpinnerClass);

            if (!LoadingSpinnerInstance)
            {
                UE_LOG(LogTemp, Error,
                       TEXT("Failed to create instance of LoadingSpinnerClass in EVAppPlayerController"));
                return;
            }
        }

        if (!LoadingSpinnerInstance->IsInViewport())
        {
            UE_LOG(LogTemp, Warning, TEXT("Created spinner"));
            LoadingSpinnerInstance->AddToViewport(9999);
        }

        return;
    }

    if (LoadingSpinnerInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Removed spinner"));
        LoadingSpinnerInstance->RemoveFromParent();
        LoadingSpinnerInstance = nullptr;
    }
}

void AEVAppPlayerController::HandleActionStatusWidget(const FEVRequestedActionInfo& RequestedActionInfo)
{
    if (RequestedActionStatusWidgetClass)
    {
        RequestedActionStatusWidgetInstance = CreateWidget<UUserWidget>(this, RequestedActionStatusWidgetClass);

        if (RequestedActionStatusWidgetInstance)
        {
            if (IEVDisplayStatusProvider* StatusDisplay =
                    Cast<IEVDisplayStatusProvider>(RequestedActionStatusWidgetInstance))
            {
                StatusDisplay->ShowStatus(RequestedActionInfo);
            }

            RequestedActionStatusWidgetInstance->AddToViewport(9999);
        }
        else
        {
            UE_LOG(LogTemp, Error,
                   TEXT("Failed to create instance of RequestedActionStatusWidgetClass in EVAppPlayerController"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("The RequestedActionStatusWidgetClass was not provided to EVAppPlayerController"));
    }
}
