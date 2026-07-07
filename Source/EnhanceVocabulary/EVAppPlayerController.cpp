// Fill out your copyright notice in the Description page of Project Settings.

#include "EVAppPlayerController.h"

#include "EVErrorProvider.h"
#include "EVGameInstance.h"
#include "EVErrorDisplayWidget.h"
#include "EVDisplayStatusProvider.h"

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
    EVGameInstance = Cast<UEVGameInstance>(GetGameInstance());
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

            WidgetCommonEvents = Cast<IEVWidgetCommonEvents>(RootWidgetInstance);

            if (WidgetCommonEvents)
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

                if (FOnWordEntryWidgetControlsActivated* WordEntryWidgetControlsActivated =
                        WidgetCommonEvents->GetCurrentWordEntryWidgetActionInfo())
                {
                    WordEntryWidgetControlsActivated->AddDynamic(this, &ThisClass::HandleWordEntryWidget);
                }
                else
                {
                    UE_LOG(LogTemp, Error,
                           TEXT("FOnWordEntryWidgetControlsActivated in EVAppPlayerController.cpp is nullptr"));
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

void AEVAppPlayerController::HandleWordEntryWidget(const FEVWordEntryActionInfo& CurrentWordEntryWidgetInfo)
{
    // Caching the FEVWordEntryActionInfo received from a particular WordEntry
    CachedWordEntryWidgetInfo = CurrentWordEntryWidgetInfo;

    // ASsigning another cached var to update the entry once we confirm EDIT operation
    CachedConfirmedWordEntry = CurrentWordEntryWidgetInfo.EntryInfo;

    if (DetailedWordEntryWidgetClass)
    {
        DetailedWordEntryWidgetInstance = CreateWidget<UUserWidget>(this, DetailedWordEntryWidgetClass);

        if (DetailedWordEntryWidgetInstance)
        {
            DetailedWordEntryWidgetInstance->AddToViewport(9999);
            DetailedWordEntryDisplay = Cast<IEVWordEntryDisplayWidgetProvider>(DetailedWordEntryWidgetInstance);

            if (DetailedWordEntryDisplay)
            {
                DetailedWordEntryDisplay->ShowWordEntry(CurrentWordEntryWidgetInfo.EntryInfo);
                DetailedWordEntryDisplay->GetViewPressedDelegate().AddUObject(
                    this, &ThisClass::HandleDetailedViewButtonPressed);
                DetailedWordEntryDisplay->GetEditPressedDelegate().AddUObject(
                    this, &ThisClass::HandleDetailedEditButtonPressed);
                DetailedWordEntryDisplay->GetDeletePressedDelegate().AddUObject(
                    this, &ThisClass::HandleDetailedDeleteButtonPressed);
                DetailedWordEntryDisplay->GetSaveChangesSubmittedDelegate().AddUObject(
                    this, &ThisClass::HandleDetailedSaveChangesButtonPressed);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error,
                   TEXT("Failed to create instance of DetailedWordEntryWidgetClass in EVAppPlayerController"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("The DetailedWordEntryWidgetClass was not provided to EVAppPlayerController"));
    }
}

void AEVAppPlayerController::HandleDetailedViewButtonPressed()
{
    if (DetailedWordEntryWidgetInstance)
    {
        HandleCreateConfirmationDialog(EEVConfirmationDialogType::ExitViewWord, EEVWordEntryActionType::CloseEntry);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DetailedWordEntryWidgetInstance is nullptr in PC"));
    }
}

void AEVAppPlayerController::HandleDetailedEditButtonPressed()
{
    if (DetailedWordEntryDisplay)
    {
        DetailedWordEntryDisplay->SetButtonsDisabled(true, true, true, false);
        DetailedWordEntryDisplay->SetEditableFieldsReadOnly(false);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DetailedWordEntryDisplay is nullptr in PC"));
    }
}

void AEVAppPlayerController::HandleDetailedSaveChangesButtonPressed(const FVocabularyEntry& NewVocabularyEntry)
{
    CachedWordEntryWidgetInfo.EntryInfo = NewVocabularyEntry;

    HandleCreateConfirmationDialog(EEVConfirmationDialogType::EditWord, EEVWordEntryActionType::SaveEditedEntry);
}

void AEVAppPlayerController::HandleDetailedDeleteButtonPressed()
{
    HandleCreateConfirmationDialog(EEVConfirmationDialogType::DeleteWord, EEVWordEntryActionType::DeleteEntry);
}

void AEVAppPlayerController::HandleCreateConfirmationDialog(EEVConfirmationDialogType DialogType,
                                                            EEVWordEntryActionType PendingActionType)
{
    CachedWordEntryWidgetInfo.ActionType = PendingActionType;

    if (!ConfirmationDialogWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("The ConfirmationDialogWidgetClass was not provided to EVAppPlayerController"));
        return;
    }

    if (ConfirmationDialogWidgetInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Confirmation dialog is already open"));
        return;
    }

    ConfirmationDialogWidgetInstance = CreateWidget<UUserWidget>(this, ConfirmationDialogWidgetClass);

    if (!ConfirmationDialogWidgetInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create instance of ConfirmationDialogWidgetClass"));
        return;
    }

    ConfirmationDialogWidgetInstance->AddToViewport(9999);

    ConfirmationDialogWidget = Cast<IEVConfirmationDialogWidgetProvider>(ConfirmationDialogWidgetInstance);

    if (!ConfirmationDialogWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("ConfirmationDialogWidget does not implement provider interface"));
        return;
    }

    FEVConfirmationDialogInfo ConfirmationDialogInfo;
    ConfirmationDialogInfo.DialogType = DialogType;
    ConfirmationDialogInfo.Generate();

    ConfirmationDialogWidget->SetConfirmationDialogInfo(ConfirmationDialogInfo);

    ConfirmationDialogWidget->GetDialogButtonPressedDelegate().RemoveAll(this);
    ConfirmationDialogWidget->GetDialogButtonPressedDelegate().AddUObject(
        this, &ThisClass::HandleConfirmationDialog_ButtonPressed);
}

void AEVAppPlayerController::HandleConfirmationDialog_ButtonPressed(bool bIsOperationConfirmed)
{
    if (ConfirmationDialogWidgetInstance)
    {
        ConfirmationDialogWidgetInstance->RemoveFromParent();
        ConfirmationDialogWidgetInstance = nullptr;
        ConfirmationDialogWidget = nullptr;
    }

    if (!bIsOperationConfirmed)
    {
        if (CachedWordEntryWidgetInfo.ActionType == EEVWordEntryActionType::SaveEditedEntry)
        {
            CachedWordEntryWidgetInfo.EntryInfo = CachedConfirmedWordEntry;

            if (DetailedWordEntryDisplay)
            {
                DetailedWordEntryDisplay->ShowWordEntry(CachedConfirmedWordEntry);
                DetailedWordEntryDisplay->SetEditableFieldsReadOnly(true);
                DetailedWordEntryDisplay->SetButtonsDisabled(false, false, false, true);
            }
        }

        return;
    }

    switch (CachedWordEntryWidgetInfo.ActionType)
    {
    case EEVWordEntryActionType::CloseEntry:
        if (DetailedWordEntryWidgetInstance)
        {
            DetailedWordEntryWidgetInstance->RemoveFromParent();
            DetailedWordEntryWidgetInstance = nullptr;
            DetailedWordEntryDisplay = nullptr;
        }
        break;

    case EEVWordEntryActionType::SaveEditedEntry:
        // Save CachedWordEntryWidgetInfo.EntryInfo here.
        // Keep DetailedWordEntryWidgetInstance alive.
        ProcessConfirmedWordUpdate();
        break;

    case EEVWordEntryActionType::DeleteEntry:
        ProcessConfirmedWordDelete();
        break;

    default:
        break;
    }
}

void AEVAppPlayerController::ProcessConfirmedWordUpdate()
{

    HandleLoadingSpinner(true);

    FVocabularyEntry UpdatedEntry;

    const bool bUpdated =
        EVGameInstance && EVGameInstance->UpdateVocabularyEntry(CachedWordEntryWidgetInfo.EntryInfo, UpdatedEntry);

    HandleLoadingSpinner(false);

    FEVRequestedActionInfo StatusInfo;
    StatusInfo.Source = EEVRequestedActionSource::ReviewWords;
    StatusInfo.Type = EEVRequestedActionType::EditWord;
    StatusInfo.Status = bUpdated ? EEVRequestedActionStatus::Saved : EEVRequestedActionStatus::Failed;
    StatusInfo.GenerateMessage();
    StatusInfo.GenerateColor();

    HandleActionStatusWidget(StatusInfo);

    if (bUpdated)
    {
        CachedConfirmedWordEntry = UpdatedEntry;

        CachedWordEntryWidgetInfo.ActionType = EEVWordEntryActionType::SaveEditedEntry;
        CachedWordEntryWidgetInfo.EntryInfo = UpdatedEntry;

        if (DetailedWordEntryDisplay)
        {
            DetailedWordEntryDisplay->ShowWordEntry(UpdatedEntry);
            DetailedWordEntryDisplay->SetEditableFieldsReadOnly(true);
            DetailedWordEntryDisplay->SetButtonsDisabled(false, false, false, true);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("DetailedWordEntryDisplay is false in ProcessConfirmedWordUpdate"));
        }

        if (WidgetCommonEvents)
        {
            WidgetCommonEvents->HandleWordEntryChanged(CachedWordEntryWidgetInfo);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("bUpdated is false in ProcessConfirmedWordUpdate"));
    }
}

void AEVAppPlayerController::ProcessConfirmedWordDelete()
{
    HandleLoadingSpinner(true);

    const bool bDeleted = EVGameInstance && EVGameInstance->DeleteVocabularyEntry(CachedWordEntryWidgetInfo.EntryInfo);

    HandleLoadingSpinner(false);

    FEVRequestedActionInfo StatusInfo;
    StatusInfo.Source = EEVRequestedActionSource::ReviewWords;
    StatusInfo.Type = EEVRequestedActionType::DeleteWord;
    StatusInfo.Status = bDeleted ? EEVRequestedActionStatus::Done : EEVRequestedActionStatus::Failed;
    StatusInfo.GenerateMessage();
    StatusInfo.GenerateColor();

    HandleActionStatusWidget(StatusInfo);

    if (!bDeleted)
    {
        return;
    }

    CachedWordEntryWidgetInfo.ActionType = EEVWordEntryActionType::DeleteEntry;

    if (WidgetCommonEvents)
    {
        WidgetCommonEvents->HandleWordEntryChanged(CachedWordEntryWidgetInfo);
    }

    DestroyWidget(DetailedWordEntryWidgetInstance);
    DetailedWordEntryDisplay = nullptr;
}

void AEVAppPlayerController::DestroyWidget(TObjectPtr<UUserWidget>& Widget)
{
    if (Widget)
    {
        Widget->RemoveFromParent();
        Widget = nullptr;
    }
}