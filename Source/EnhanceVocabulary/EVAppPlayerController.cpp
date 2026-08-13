// Fill out your copyright notice in the Description page of Project Settings.

#include "EVAppPlayerController.h"

#include "EVErrorProvider.h"
#include "EVGameInstance.h"
#include "EVErrorDisplayWidget.h"
#include "EVDisplayStatusProvider.h"
#include "Misc/CoreDelegates.h"
#include "TimerManager.h"

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

    EVGameInstance = Cast<UEVGameInstance>(GetGameInstance());
    InitEVAppPlayerController();
    if (EVGameInstance)
    {
        EVGameInstance->OnFileOperationCompleted().AddUObject(this, &ThisClass::HandleFileOperationCompleted);
        EVGameInstance->OnImportFilePickCompleted().AddUObject(this, &ThisClass::HandleImportFilePickCompleted);
        EVGameInstance->OnNotificationPermissionResult().AddUObject(this,
                                                                    &ThisClass::HandleNotificationPermissionResult);

#if PLATFORM_ANDROID

        SynchronizeNotificationSettingsFromDevice();
        HandlePendingNotificationWord();

        GetWorldTimerManager().SetTimer(NotificationStatePollTimerHandle, this, &ThisClass::PollNotificationState, 0.5f,
                                        true);
#endif
    }
    ApplicationEnteredForegroundHandle = FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddUObject(
        this, &ThisClass::HandleApplicationEnteredForeground);
}

void AEVAppPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(NotificationStatePollTimerHandle);

    UnbindApplicationPorts();

    if (EVGameInstance)
    {
        EVGameInstance->OnNotificationPermissionResult().RemoveAll(this);

        EVGameInstance->OnFileOperationCompleted().RemoveAll(this);

        EVGameInstance->OnImportFilePickCompleted().RemoveAll(this);
    }

    if (ApplicationEnteredForegroundHandle.IsValid())
    {
        FCoreDelegates::ApplicationHasEnteredForegroundDelegate.Remove(ApplicationEnteredForegroundHandle);
    }

    Super::EndPlay(EndPlayReason);
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

                if (FOnImportExportDownloadDBOperationIssued* ImportExportDownloadDBOperationIssued =
                        WidgetCommonEvents->GetIssuedFileOperationInfo())
                {
                    ImportExportDownloadDBOperationIssued->AddDynamic(this, &ThisClass::HandleIssuedFileOperation);
                }
                else
                {
                    UE_LOG(LogTemp, Error,
                           TEXT("FOnImportExportDownloadDBOperationIssued in EVAppPlayerController.cpp is nullptr"));
                }

                if (FOnPopUpIntervalSelectedFromSettings* PopUpIntervalSelectedFromSettings =
                        WidgetCommonEvents->GetSelectedPopUpInterval())
                {
                    PopUpIntervalSelectedFromSettings->AddDynamic(this, &ThisClass::HandleNotificationSettingsChanged);
                }
                else
                {
                    UE_LOG(LogTemp, Error,
                           TEXT("FOnImportExportDownloadDBOperationIssued in EVAppPlayerController.cpp is nullptr"));
                }

                if (FOnVocabularyValueActionRequested* VocabularyValueActionRequested =
                        WidgetCommonEvents->GetVocabularyValueActionRequestedEvent())
                {
                    VocabularyValueActionRequested->AddDynamic(this, &ThisClass::HandleVocabularyValueActionRequested);
                }
                else
                {
                    UE_LOG(LogTemp, Error,
                           TEXT("FOnVocabularyValueActionRequested in EVAppPlayerController.cpp is nullptr"));
                }

                if (FOnVocabularyFiltersRequested* VocabularyFiltersRequested =
                        WidgetCommonEvents->GetVocabularyFiltersRequestedEvent())
                {
                    VocabularyFiltersRequested->AddDynamic(this, &ThisClass::HandleVocabularyFiltersRequested);
                }
                else
                {
                    UE_LOG(LogTemp, Error,
                           TEXT("FOnVocabularyFiltersRequested in EVAppPlayerController.cpp is nullptr"));
                }

                if (FOnVocabularyLanguagePreferencesChangedFromWidgets* LanguagePreferencesChanged =
                        WidgetCommonEvents->GetVocabularyLanguagePreferencesChangedEvent())
                {
                    LanguagePreferencesChanged->AddDynamic(this,
                                                           &ThisClass::HandleVocabularyLanguagePreferencesChanged);
                }
                else
                {
                    UE_LOG(LogTemp, Error,
                           TEXT("FOnVocabularyLanguagePreferencesChangedFromWidgets in EVAppPlayerController.cpp is "
                                "nullptr"));
                }

                if (EVGameInstance)
                {
                    WidgetCommonEvents->HandleVocabularyLanguagePreferencesApplied(
                        EVGameInstance->GetVocabularyLanguagePreferences());
                }
            }
            else
            {
                UE_LOG(LogTemp, Error,
                       TEXT("Failed to create instance of IEVWidgetCommonEvents in EVAppPlayerController"));
            }

            BindApplicationPorts();
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

void AEVAppPlayerController::BindApplicationPorts()
{
    UnbindApplicationPorts();

    if (!RootWidgetInstance)
    {
        return;
    }

    VocabularySearchApplicationPort = Cast<IEVVocabularySearchApplicationPort>(RootWidgetInstance);
    VocabularyLibraryApplicationPort = Cast<IEVVocabularyLibraryApplicationPort>(RootWidgetInstance);
    VocabularyPreferencesApplicationPort = Cast<IEVVocabularyPreferencesApplicationPort>(RootWidgetInstance);
    NetworkConnectivityApplicationPort = Cast<IEVNetworkConnectivityApplicationPort>(RootWidgetInstance);
    GlobalPresentationResolutionPort = Cast<IEVGlobalPresentationResolutionPort>(RootWidgetInstance);

    if (VocabularySearchApplicationPort)
    {
        VocabularySearchApplicationPort->GetVocabularySearchRequestedEvent().AddUObject(
            this, &ThisClass::HandleVocabularySearchRequested);
    }

    if (VocabularyLibraryApplicationPort)
    {
        VocabularyLibraryApplicationPort->GetVocabularyRecordRequestedEvent().AddUObject(
            this, &ThisClass::HandleVocabularyRecordRequested);
        VocabularyLibraryApplicationPort->GetVocabularyQueryRequestedEvent().AddUObject(
            this, &ThisClass::HandleVocabularyQueryRequested);
        VocabularyLibraryApplicationPort->GetVocabularyMutationRequestedEvent().AddUObject(
            this, &ThisClass::HandleVocabularyMutationRequested);
    }

    if (VocabularyPreferencesApplicationPort)
    {
        VocabularyPreferencesApplicationPort->GetVocabularyPreferencesChangeRequestedEvent().AddUObject(
            this, &ThisClass::HandleVocabularyPreferencesChangeRequested);
    }

    if (EVGameInstance)
    {
        EVGameInstance->OnVocabularySearchOutcomeReady().AddUObject(this,
                                                                    &ThisClass::HandleVocabularySearchOutcomeReady);
        EVGameInstance->OnVocabularyRecordOutcomeReady().AddUObject(this,
                                                                    &ThisClass::HandleVocabularyRecordOutcomeReady);
        EVGameInstance->OnVocabularyQueryOutcomeReady().AddUObject(this, &ThisClass::HandleVocabularyQueryOutcomeReady);
        EVGameInstance->OnVocabularyMutationOutcomeReady().AddUObject(this,
                                                                      &ThisClass::HandleVocabularyMutationOutcomeReady);
        EVGameInstance->OnVocabularyChanged().AddUObject(this, &ThisClass::HandleVocabularyChanged);
        EVGameInstance->OnVocabularyPreferencesStateReady().AddUObject(
            this, &ThisClass::HandleVocabularyPreferencesStateReady);
    }
}

void AEVAppPlayerController::UnbindApplicationPorts()
{
    if (VocabularySearchApplicationPort)
    {
        VocabularySearchApplicationPort->GetVocabularySearchRequestedEvent().RemoveAll(this);
    }

    if (VocabularyLibraryApplicationPort)
    {
        VocabularyLibraryApplicationPort->GetVocabularyRecordRequestedEvent().RemoveAll(this);
        VocabularyLibraryApplicationPort->GetVocabularyQueryRequestedEvent().RemoveAll(this);
        VocabularyLibraryApplicationPort->GetVocabularyMutationRequestedEvent().RemoveAll(this);
    }

    if (VocabularyPreferencesApplicationPort)
    {
        VocabularyPreferencesApplicationPort->GetVocabularyPreferencesChangeRequestedEvent().RemoveAll(this);
    }

    if (EVGameInstance)
    {
        EVGameInstance->OnVocabularySearchOutcomeReady().RemoveAll(this);
        EVGameInstance->OnVocabularyRecordOutcomeReady().RemoveAll(this);
        EVGameInstance->OnVocabularyQueryOutcomeReady().RemoveAll(this);
        EVGameInstance->OnVocabularyMutationOutcomeReady().RemoveAll(this);
        EVGameInstance->OnVocabularyChanged().RemoveAll(this);
        EVGameInstance->OnVocabularyPreferencesStateReady().RemoveAll(this);
    }

    VocabularySearchApplicationPort = nullptr;
    VocabularyLibraryApplicationPort = nullptr;
    VocabularyPreferencesApplicationPort = nullptr;
    NetworkConnectivityApplicationPort = nullptr;
    GlobalPresentationResolutionPort = nullptr;
}

void AEVAppPlayerController::HandleVocabularySearchRequested(const FEVVocabularySearchRequest& Request)
{
    if (EVGameInstance)
    {
        EVGameInstance->RequestVocabularySearch(Request);
        return;
    }

    if (VocabularySearchApplicationPort)
    {
        FEVVocabularySearchOutcome Outcome;
        Outcome.RequestId = Request.RequestId;
        Outcome.Result = EEVApplicationOperationResult::Unavailable;
        Outcome.Message = FText::FromString(TEXT("Application services are unavailable."));
        VocabularySearchApplicationPort->ApplyVocabularySearchOutcome(Outcome);
    }
}

void AEVAppPlayerController::HandleVocabularyRecordRequested(const FEVVocabularyRecordRequest& Request)
{
    if (EVGameInstance)
    {
        EVGameInstance->RequestVocabularyRecord(Request);
        return;
    }

    if (VocabularyLibraryApplicationPort)
    {
        FEVVocabularyRecordOutcome Outcome;
        Outcome.RequestId = Request.RequestId;
        Outcome.Result = EEVApplicationOperationResult::Unavailable;
        Outcome.Message = FText::FromString(TEXT("Application services are unavailable."));
        VocabularyLibraryApplicationPort->ApplyVocabularyRecordOutcome(Outcome);
    }
}

void AEVAppPlayerController::HandleVocabularyQueryRequested(const FEVVocabularyQueryRequest& Request)
{
    if (EVGameInstance)
    {
        EVGameInstance->RequestVocabularyQuery(Request);
        return;
    }

    if (VocabularyLibraryApplicationPort)
    {
        FEVVocabularyQueryOutcome Outcome;
        Outcome.RequestId = Request.RequestId;
        Outcome.Result = EEVApplicationOperationResult::Unavailable;
        Outcome.Message = FText::FromString(TEXT("Application services are unavailable."));
        VocabularyLibraryApplicationPort->ApplyVocabularyQueryOutcome(Outcome);
    }
}

void AEVAppPlayerController::HandleVocabularyMutationRequested(const FEVVocabularyMutationRequest& Request)
{
    if (EVGameInstance)
    {
        EVGameInstance->RequestVocabularyMutation(Request);
        return;
    }

    if (VocabularyLibraryApplicationPort)
    {
        FEVVocabularyMutationOutcome Outcome;
        Outcome.RequestId = Request.RequestId;
        Outcome.MutationType = Request.MutationType;
        Outcome.Record = Request.Record;
        Outcome.Result = EEVApplicationOperationResult::Unavailable;
        Outcome.Message = FText::FromString(TEXT("Application services are unavailable."));
        VocabularyLibraryApplicationPort->ApplyVocabularyMutationOutcome(Outcome);
    }
}

void AEVAppPlayerController::HandleVocabularyPreferencesChangeRequested(
    const FEVVocabularyPreferencesChangeRequest& Request)
{
    if (EVGameInstance)
    {
        EVGameInstance->RequestVocabularyPreferencesChange(Request);
        return;
    }

    if (VocabularyPreferencesApplicationPort)
    {
        FEVVocabularyPreferencesState State;
        State.RequestId = Request.RequestId;
        State.Result = EEVApplicationOperationResult::Unavailable;
        State.Message = FText::FromString(TEXT("Application services are unavailable."));
        VocabularyPreferencesApplicationPort->ApplyVocabularyPreferencesState(State);
    }
}

void AEVAppPlayerController::HandleVocabularySearchOutcomeReady(const FEVVocabularySearchOutcome& Outcome)
{
    if (VocabularySearchApplicationPort)
    {
        VocabularySearchApplicationPort->ApplyVocabularySearchOutcome(Outcome);
    }
}

void AEVAppPlayerController::HandleVocabularyRecordOutcomeReady(const FEVVocabularyRecordOutcome& Outcome)
{
    if (VocabularyLibraryApplicationPort)
    {
        VocabularyLibraryApplicationPort->ApplyVocabularyRecordOutcome(Outcome);
    }
}

void AEVAppPlayerController::HandleVocabularyQueryOutcomeReady(const FEVVocabularyQueryOutcome& Outcome)
{
    if (VocabularyLibraryApplicationPort)
    {
        VocabularyLibraryApplicationPort->ApplyVocabularyQueryOutcome(Outcome);
    }
}

void AEVAppPlayerController::HandleVocabularyMutationOutcomeReady(const FEVVocabularyMutationOutcome& Outcome)
{
    if (VocabularyLibraryApplicationPort)
    {
        VocabularyLibraryApplicationPort->ApplyVocabularyMutationOutcome(Outcome);
    }
}

void AEVAppPlayerController::HandleVocabularyChanged(const FEVVocabularyChangeInfo& ChangeInfo)
{
    if (VocabularyLibraryApplicationPort)
    {
        VocabularyLibraryApplicationPort->ApplyVocabularyChanged(ChangeInfo);
    }
}

void AEVAppPlayerController::HandleVocabularyPreferencesStateReady(const FEVVocabularyPreferencesState& State)
{
    if (VocabularyPreferencesApplicationPort)
    {
        VocabularyPreferencesApplicationPort->ApplyVocabularyPreferencesState(State);
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
                EVErrorInfo = WidgetErrorInfo;
                ErrorDisplay->ShowError(EVErrorInfo.Message);
                ErrorDisplay->OnErrorWidgetDestroyed().AddDynamic(this, &ThisClass::HandleErrorWidgetDestroyed);
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

void AEVAppPlayerController::HandleErrorWidgetDestroyed()
{
    OnWidgetsErrorResolved.Broadcast(EVErrorInfo);
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
    CachedWordEntryWidgetInfo = CurrentWordEntryWidgetInfo;

    if (!EVGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("EVGameInstance is nullptr in HandleWordEntryWidget"));
        return;
    }

    FEVVocabularyRecord VocabularyRecord;

    if (!EVGameInstance->GetVocabularyRecordByWord(CurrentWordEntryWidgetInfo.EntryInfo.NormalizedWord.IsEmpty()
                                                       ? CurrentWordEntryWidgetInfo.EntryInfo.Word
                                                       : CurrentWordEntryWidgetInfo.EntryInfo.NormalizedWord,
                                                   VocabularyRecord))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load structured vocabulary record for Detailed View: %s"),
               *CurrentWordEntryWidgetInfo.EntryInfo.Word);
        return;
    }

    CachedConfirmedVocabularyRecord = VocabularyRecord;
    CachedPendingVocabularyRecord = FEVVocabularyRecord{};

    if (DetailedWordEntryWidgetClass)
    {
        DetailedWordEntryWidgetInstance = CreateWidget<UUserWidget>(this, DetailedWordEntryWidgetClass);

        if (DetailedWordEntryWidgetInstance)
        {
            DetailedWordEntryWidgetInstance->AddToViewport(9999);
            DetailedWordEntryDisplay = Cast<IEVWordEntryDisplayWidgetProvider>(DetailedWordEntryWidgetInstance);

            if (DetailedWordEntryDisplay)
            {
                DetailedWordEntryDisplay->ShowWordEntry(CachedConfirmedVocabularyRecord);
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

// Will try to control the action that came from widget (ImportExportDB in this case)
// from the PC rather than on the widget's side. If this approach proves to be cleaner than
// what we have now we'll try to refactor other parts of the app to implement it.
void AEVAppPlayerController::HandleIssuedFileOperation(const FEVFileOperationInfo& IssuedFileOperation)
{
    if (!EVGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("EVGameInstance is nullptr in EVAppPlayerController"));

        return;
    }

    switch (IssuedFileOperation.OperationType)
    {
    case EEVFileOperationType::ImportDBOverwrite:
    {
        PendingFileOperationInfo = IssuedFileOperation;

        // Do not show the spinner while waiting for confirmation.
        HandleCreateConfirmationDialog(EEVConfirmationDialogType::OverwriteDB, EEVWordEntryActionType::Unknown);

        return;
    }
    case EEVFileOperationType::ImportDBAppend:
    {
        PendingFileOperationInfo = IssuedFileOperation;

        HandleCreateConfirmationDialog(EEVConfirmationDialogType::AppendDB, EEVWordEntryActionType::Unknown);

        return;
    }

    default:
        break;
    }

    // Existing Download Template / Export flow.
    HandleLoadingSpinner(true);

    const FEVRequestedActionInfo RequestedActionInfo =
        EVGameInstance->HandleFileOperationRequested(IssuedFileOperation);

    if (RequestedActionInfo.Status != EEVRequestedActionStatus::InProgress)
    {
        HandleLoadingSpinner(false);
        HandleActionStatusWidget(RequestedActionInfo);
    }
}

void AEVAppPlayerController::HandleFileOperationCompleted(const FEVRequestedActionInfo& RequestedActionInfo)
{
    HandleLoadingSpinner(false);
    HandleActionStatusWidget(RequestedActionInfo);
}

void AEVAppPlayerController::HandleImportFilePickCompleted(const FEVFileExchangeResultInfo& ResultInfo)
{
    HandleLoadingSpinner(false);

    if (ResultInfo.Result == EEVFileExchangeResult::CancelledByUser)
    {
        UE_LOG(LogTemp, Warning, TEXT("Import file selection cancelled by user."));

        PendingFileOperationInfo = FEVFileOperationInfo();
        return;
    }

    if (!ResultInfo.IsSuccess())
    {
        FEVRequestedActionInfo ActionInfo;
        ActionInfo.Source = EEVRequestedActionSource::ImportExport;
        ActionInfo.Type = EEVRequestedActionType::ImportDBOverwrite;
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(ResultInfo.UserMessage);
        ActionInfo.GenerateColor();

        HandleActionStatusWidget(ActionInfo);

        PendingFileOperationInfo = FEVFileOperationInfo();
        return;
    }

    FEVRequestedActionInfo ActionInfo;

    ActionInfo.Source = EEVRequestedActionSource::ImportExport;

    ActionInfo.Type = EEVRequestedActionType::ImportDBOverwrite;

    ActionInfo.Status = EEVRequestedActionStatus::Completed;

    ActionInfo.Message = FText::FromString(ResultInfo.UserMessage);

    ActionInfo.GenerateColor();

    HandleActionStatusWidget(ActionInfo);

    PendingFileOperationInfo = FEVFileOperationInfo();

    if (WidgetCommonEvents)
    {
        WidgetCommonEvents->HandleReviewWordsRefresh();
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

void AEVAppPlayerController::HandleDetailedSaveChangesButtonPressed(const FEVVocabularyRecord& NewVocabularyRecord)
{
    CachedPendingVocabularyRecord = NewVocabularyRecord;

    HandleCreateConfirmationDialog(EEVConfirmationDialogType::EditWord, EEVWordEntryActionType::SaveEditedEntry);
}

void AEVAppPlayerController::HandleDetailedDeleteButtonPressed()
{
    HandleCreateConfirmationDialog(EEVConfirmationDialogType::DeleteWord, EEVWordEntryActionType::DeleteEntry);
}

void AEVAppPlayerController::HandleCreateConfirmationDialog(EEVConfirmationDialogType DialogType,
                                                            EEVWordEntryActionType PendingActionType)
{
    PendingConfirmationDialogType = DialogType;

    CachedWordEntryWidgetInfo.ActionType = PendingActionType;

    // Existing implementation continues unchanged...

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
    if (DialogType == EEVConfirmationDialogType::ReviewExistingRelatedWord ||
        DialogType == EEVConfirmationDialogType::AddMissingRelatedWord)
    {
        ConfirmationDialogInfo.SubjectValue = PendingVocabularyValueActionRequest.Relation.RelatedWord;
    }
    else if (DialogType == EEVConfirmationDialogType::UnsupportedTranslationLanguage ||
             DialogType == EEVConfirmationDialogType::CreateTranslationLanguageContext)
    {
        EEVVocabularyTranslationLanguage Language = EEVVocabularyTranslationLanguage::None;
        ConfirmationDialogInfo.SubjectValue =
            EVVocabularyLanguage::TryParseTranslationLanguage(
                PendingVocabularyValueActionRequest.Translation.TargetLanguage, Language)
                ? EVVocabularyLanguage::GetTranslationLanguageDisplayText(Language).ToString()
                : PendingVocabularyValueActionRequest.Translation.TargetLanguage;
    }
    else if (DialogType == EEVConfirmationDialogType::ReviewExistingTranslationWord ||
             DialogType == EEVConfirmationDialogType::AddMissingTranslationWord)
    {
        ConfirmationDialogInfo.SubjectValue = PendingVocabularyValueActionRequest.Translation.TranslationText;
    }
    ConfirmationDialogInfo.Generate();

    ConfirmationDialogWidget->SetConfirmationDialogInfo(ConfirmationDialogInfo);

    ConfirmationDialogWidget->GetDialogButtonPressedDelegate().RemoveAll(this);
    ConfirmationDialogWidget->GetDialogButtonPressedDelegate().AddUObject(
        this, &ThisClass::HandleConfirmationDialog_ButtonPressed);
}

void AEVAppPlayerController::HandleConfirmationDialog_ButtonPressed(bool bIsOperationConfirmed)
{
    const EEVConfirmationDialogType ConfirmedDialogType = PendingConfirmationDialogType;

    PendingConfirmationDialogType = EEVConfirmationDialogType::Unknown;

    if (ConfirmationDialogWidgetInstance)
    {
        ConfirmationDialogWidgetInstance->RemoveFromParent();
        ConfirmationDialogWidgetInstance = nullptr;
        ConfirmationDialogWidget = nullptr;
    }

    /*
     * Change notification mode
     */
    if (ConfirmedDialogType == EEVConfirmationDialogType::ReviewExistingRelatedWord)
    {
        if (bIsOperationConfirmed && WidgetCommonEvents)
        {
            WidgetCommonEvents->HandleOpenReviewWordsForNotification(
                PendingVocabularyValueActionRequest.Relation.RelatedWord);
        }
        PendingVocabularyValueActionRequest = FEVVocabularyValueActionRequest{};
        return;
    }

    if (ConfirmedDialogType == EEVConfirmationDialogType::AddMissingRelatedWord)
    {
        if (bIsOperationConfirmed && WidgetCommonEvents)
        {
            WidgetCommonEvents->HandleOpenAddWordWithWord(PendingVocabularyValueActionRequest.Relation.RelatedWord);
        }
        PendingVocabularyValueActionRequest = FEVVocabularyValueActionRequest{};
        return;
    }

    if (ConfirmedDialogType == EEVConfirmationDialogType::UnsupportedTranslationLanguage ||
        ConfirmedDialogType == EEVConfirmationDialogType::CreateTranslationLanguageContext ||
        ConfirmedDialogType == EEVConfirmationDialogType::ReviewExistingTranslationWord ||
        ConfirmedDialogType == EEVConfirmationDialogType::AddMissingTranslationWord)
    {
        if (bIsOperationConfirmed)
        {
            if (ConfirmedDialogType == EEVConfirmationDialogType::CreateTranslationLanguageContext)
            {
                HandleTranslationContextCreationConfirmed();
            }
            else if (ConfirmedDialogType == EEVConfirmationDialogType::ReviewExistingTranslationWord)
            {
                HandleTranslationExistingWordConfirmed();
            }
            else if (ConfirmedDialogType == EEVConfirmationDialogType::AddMissingTranslationWord)
            {
                HandleTranslationMissingWordConfirmed();
            }
        }
        PendingVocabularyValueActionRequest = FEVVocabularyValueActionRequest{};
        return;
    }

    if (ConfirmedDialogType == EEVConfirmationDialogType::ChangeNotificationMode)
    {
        if (bIsOperationConfirmed)
        {
            CommitPendingModeChange();
        }
        else
        {
            RejectPendingModeChange();
        }

        return;
    }

    /*
     * Enable Android notifications
     */
    if (ConfirmedDialogType == EEVConfirmationDialogType::EnableAndroidNotifications)
    {
        if (!bIsOperationConfirmed)
        {
            RejectPendingNotificationRequest();
            return;
        }

        if (!EVGameInstance)
        {
            UE_LOG(LogTemp, Error, TEXT("Cannot open notification settings: EVGameInstance is nullptr."));

            RejectPendingNotificationRequest();
            return;
        }

        bWaitingForNotificationSettings = true;

        EVGameInstance->OpenNotificationSettings();

        return;
    }

    /*
     * Import DB — Overwrite
     */
    if (ConfirmedDialogType == EEVConfirmationDialogType::OverwriteDB)
    {
        if (!bIsOperationConfirmed)
        {
            UE_LOG(LogTemp, Warning, TEXT("Import DB overwrite cancelled by user."));

            PendingFileOperationInfo = FEVFileOperationInfo();
            return;
        }

        if (PendingFileOperationInfo.OperationType != EEVFileOperationType::ImportDBOverwrite)
        {
            UE_LOG(LogTemp, Error, TEXT("Overwrite confirmation received without a pending overwrite operation."));

            PendingFileOperationInfo = FEVFileOperationInfo();
            return;
        }

        if (!EVGameInstance)
        {
            UE_LOG(LogTemp, Error, TEXT("Cannot start overwrite import: EVGameInstance is null."));

            PendingFileOperationInfo = FEVFileOperationInfo();
            return;
        }

        UE_LOG(LogTemp, Warning, TEXT("Import DB overwrite confirmed. Extension: %d"),
               static_cast<int32>(PendingFileOperationInfo.FileExtensionType));

        HandleLoadingSpinner(true);

        const FEVRequestedActionInfo RequestedActionInfo =
            EVGameInstance->HandleFileOperationRequested(PendingFileOperationInfo);

        // Start before invoking GameInstance.
        //
        // Editor picker is synchronous, so its completion callback may execute
        // before HandleFileOperationRequested() returns.
        //
        // Android will later be asynchronous, and the same spinner will remain
        // visible until its callback arrives.

        if (RequestedActionInfo.Status != EEVRequestedActionStatus::InProgress)
        {
            HandleLoadingSpinner(false);
            HandleActionStatusWidget(RequestedActionInfo);

            PendingFileOperationInfo = FEVFileOperationInfo();
        }

        return;
    }

    /*
     * Import DB — Append
     */
    /*
     * Import DB — Append
     */
    if (ConfirmedDialogType == EEVConfirmationDialogType::AppendDB)
    {
        if (!bIsOperationConfirmed)
        {
            UE_LOG(LogTemp, Warning, TEXT("Import DB append cancelled by user."));

            PendingFileOperationInfo = FEVFileOperationInfo();
            return;
        }

        if (PendingFileOperationInfo.OperationType != EEVFileOperationType::ImportDBAppend)
        {
            UE_LOG(LogTemp, Error, TEXT("Append confirmation received without a pending append operation."));

            PendingFileOperationInfo = FEVFileOperationInfo();
            return;
        }

        if (!EVGameInstance)
        {
            UE_LOG(LogTemp, Error, TEXT("Cannot start append import: EVGameInstance is null."));

            PendingFileOperationInfo = FEVFileOperationInfo();
            return;
        }

        UE_LOG(LogTemp, Warning, TEXT("Import DB append confirmed. Extension: %d"),
               static_cast<int32>(PendingFileOperationInfo.FileExtensionType));

        HandleLoadingSpinner(true);

        const FEVRequestedActionInfo RequestedActionInfo =
            EVGameInstance->HandleFileOperationRequested(PendingFileOperationInfo);

        if (RequestedActionInfo.Status != EEVRequestedActionStatus::InProgress)
        {
            HandleLoadingSpinner(false);
            HandleActionStatusWidget(RequestedActionInfo);

            PendingFileOperationInfo = FEVFileOperationInfo();
        }

        return;
    }

    /*
     * Existing word-entry confirmation flow
     */
    if (!bIsOperationConfirmed)
    {
        if (CachedWordEntryWidgetInfo.ActionType == EEVWordEntryActionType::SaveEditedEntry)
        {
            CachedPendingVocabularyRecord = FEVVocabularyRecord{};

            if (DetailedWordEntryDisplay)
            {
                DetailedWordEntryDisplay->ShowWordEntry(CachedConfirmedVocabularyRecord);

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

    FEVVocabularyRecord UpdatedRecord;

    const bool bUpdated =
        EVGameInstance && EVGameInstance->UpdateVocabularyRecord(CachedPendingVocabularyRecord, UpdatedRecord);

    HandleLoadingSpinner(false);

    FEVRequestedActionInfo StatusInfo;
    StatusInfo.Source = EEVRequestedActionSource::ReviewWords;
    StatusInfo.Type = EEVRequestedActionType::EditWord;
    StatusInfo.Status = bUpdated ? EEVRequestedActionStatus::Saved : EEVRequestedActionStatus::Failed;
    StatusInfo.GenerateMessage();
    StatusInfo.GenerateColor();

    HandleActionStatusWidget(StatusInfo);

    if (!bUpdated)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to update structured vocabulary record."));
        return;
    }

    CachedConfirmedVocabularyRecord = UpdatedRecord;
    CachedPendingVocabularyRecord = FEVVocabularyRecord{};

    FVocabularyEntry UpdatedLegacyEntry;

    if (!EVGameInstance->GetVocabularyEntryByWord(UpdatedRecord.NormalizedWord, UpdatedLegacyEntry))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load compatibility entry after structured update: %s"),
               *UpdatedRecord.Word);
        return;
    }

    CachedWordEntryWidgetInfo.ActionType = EEVWordEntryActionType::SaveEditedEntry;
    CachedWordEntryWidgetInfo.EntryInfo = UpdatedLegacyEntry;

    if (DetailedWordEntryDisplay)
    {
        DetailedWordEntryDisplay->ShowWordEntry(CachedConfirmedVocabularyRecord);
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

void AEVAppPlayerController::HandleNotificationSettingsChanged(const FEVPopUpSettingsInfo& RequestedSettings)
{
    if (bNotificationTransitionInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Ignoring notification settings change while another transition is pending."));
        ApplyResolvedNotificationSettings(CurrentAcceptedSettings);
        return;
    }

    PendingRequestedSettings = RequestedSettings;
    bHasPendingRequestedSettings = true;
    bNotificationTransitionInProgress = true;

    EvaluateNotificationSettingsChange();
}

void AEVAppPlayerController::EvaluateNotificationSettingsChange()
{
    if (!bHasPendingRequestedSettings)
    {
        bNotificationTransitionInProgress = false;
        return;
    }

    const bool bIntervalChanged = HasNotificationIntervalChanged();
    const bool bModeChanged = HasNotificationModeChanged();

    if (!bIntervalChanged && !bModeChanged)
    {
        ClearPendingNotificationRequest();
        ApplyResolvedNotificationSettings(CurrentAcceptedSettings);
        return;
    }

    if (bModeChanged)
    {
        switch (PendingRequestedSettings.NotificationMode)
        {
        case EEVNotificationMode::RandomWord:
            EvaluateRandomWordModeChange();
            return;

        case EEVNotificationMode::TestMode:
            EvaluateTestModeChange();
            return;

        default:
            ensureMsgf(false, TEXT("Unsupported notification mode."));
            RejectPendingModeChange();
            return;
        }
    }

    HandleNotificationIntervalChange();
}

void AEVAppPlayerController::EvaluateRandomWordModeChange()
{
    RequestNotificationModeChange();
}

void AEVAppPlayerController::EvaluateTestModeChange()
{
    RequestNotificationModeChange();
}

void AEVAppPlayerController::HandleNotificationIntervalChange()
{
    if (!bHasPendingRequestedSettings)
    {
        ClearPendingNotificationRequest();
        return;
    }

    ProcessNotificationSettingsRequest(PendingRequestedSettings);
}

void AEVAppPlayerController::RequestNotificationModeChange()
{
    if (!bHasPendingRequestedSettings)
    {
        ClearPendingNotificationRequest();
        return;
    }

    HandleCreateConfirmationDialog(EEVConfirmationDialogType::ChangeNotificationMode, EEVWordEntryActionType::Unknown);
}

void AEVAppPlayerController::CommitPendingModeChange()
{
    if (!bHasPendingRequestedSettings)
    {
        ClearPendingNotificationRequest();
        ApplyResolvedNotificationSettings(CurrentAcceptedSettings);
        return;
    }

    if (!EVGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot change notification mode: EVGameInstance is nullptr."));
        RejectPendingModeChange();
        return;
    }

    FEVPopUpSettingsInfo ResolvedSettings = PendingRequestedSettings;
    ResolvedSettings.PopUpIntervals = EEVPopUpIntervals::TurnedOff;

    if (!EVGameInstance->HandlePopUpIntervalSelected(ResolvedSettings))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to stop the notification schedule while changing mode."));
        RejectPendingModeChange();
        return;
    }

    CurrentAcceptedSettings = ResolvedSettings;
    ClearPendingNotificationRequest();
    ApplyResolvedNotificationSettings(CurrentAcceptedSettings);

    FEVRequestedActionInfo ActionInfo;
    ActionInfo.Source = EEVRequestedActionSource::PopUpSettings;
    ActionInfo.Type = EEVRequestedActionType::EnableNotifications;
    ActionInfo.Status = EEVRequestedActionStatus::Warning;
    ActionInfo.Message =
        FText::FromString(TEXT("Notification mode changed. Select a notification interval to start the new mode."));
    ActionInfo.GenerateColor();
    HandleActionStatusWidget(ActionInfo);
}

void AEVAppPlayerController::RejectPendingModeChange()
{
    ClearPendingNotificationRequest();
    ApplyResolvedNotificationSettings(CurrentAcceptedSettings);
}

void AEVAppPlayerController::ProcessNotificationSettingsRequest(const FEVPopUpSettingsInfo& RequestedSettings)
{
    if (!EVGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot process notification settings: EVGameInstance is nullptr."));
        RejectPendingNotificationRequest();
        return;
    }

    const EEVPopUpIntervals SelectedInterval = RequestedSettings.PopUpIntervals;

    if (SelectedInterval == EEVPopUpIntervals::TurnedOff)
    {
        if (!EVGameInstance->HandlePopUpIntervalSelected(RequestedSettings))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to stop the notification schedule."));
            RejectPendingNotificationRequest();
            return;
        }

        CurrentAcceptedSettings = RequestedSettings;
        ClearPendingNotificationRequest();
        ApplyResolvedNotificationSettings(CurrentAcceptedSettings);
        return;
    }

    PendingPermissionSettings = RequestedSettings;
    bHasPendingPermissionSettings = true;

    FEVPopUpSettingsInfo InactiveSettings = RequestedSettings;
    InactiveSettings.PopUpIntervals = EEVPopUpIntervals::TurnedOff;
    ApplyResolvedNotificationSettings(InactiveSettings);

    if (EVGameInstance->AreNotificationsEnabled())
    {
        CommitPendingPermissionSettings();
        return;
    }

    if (!EVGameInstance->HasRequestedNotificationPermission())
    {
        if (!EVGameInstance->RequestNotificationPermission())
        {
            RejectPendingNotificationRequest();
        }

        return;
    }

    HandleCreateConfirmationDialog(EEVConfirmationDialogType::EnableAndroidNotifications,
                                   EEVWordEntryActionType::Unknown);
}

void AEVAppPlayerController::ApplyResolvedNotificationSettings(const FEVPopUpSettingsInfo& ResolvedSettings)
{
    if (!WidgetCommonEvents)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot update notification settings: WidgetCommonEvents is nullptr."));
        return;
    }

    WidgetCommonEvents->HandleApplyResolvedPopUpSettings(ResolvedSettings);
}

void AEVAppPlayerController::CommitPendingPermissionSettings()
{
    if (!bHasPendingPermissionSettings)
    {
        return;
    }

    if (!EVGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot commit notification settings: EVGameInstance is nullptr."));
        RejectPendingNotificationRequest();
        return;
    }

    const FEVPopUpSettingsInfo SettingsToCommit = PendingPermissionSettings;

    if (!EVGameInstance->HandlePopUpIntervalSelected(SettingsToCommit))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to apply the pending notification settings."));
        RejectPendingNotificationRequest();
        return;
    }

    CurrentAcceptedSettings = SettingsToCommit;
    ClearPendingNotificationRequest();
    ApplyResolvedNotificationSettings(CurrentAcceptedSettings);
}

void AEVAppPlayerController::RejectPendingNotificationRequest()
{
    bWaitingForNotificationSettings = false;
    bHasPendingPermissionSettings = false;
    PendingPermissionSettings = FEVPopUpSettingsInfo();

    FEVPopUpSettingsInfo DisabledSettings = CurrentAcceptedSettings;

    if (bHasPendingRequestedSettings)
    {
        DisabledSettings.NotificationMode = PendingRequestedSettings.NotificationMode;
    }

    DisabledSettings.PopUpIntervals = EEVPopUpIntervals::TurnedOff;

    if (EVGameInstance)
    {
        EVGameInstance->HandlePopUpIntervalSelected(DisabledSettings);
    }

    CurrentAcceptedSettings = DisabledSettings;
    ClearPendingNotificationRequest();
    ApplyResolvedNotificationSettings(CurrentAcceptedSettings);
}

void AEVAppPlayerController::ClearPendingNotificationRequest()
{
    bHasPendingRequestedSettings = false;
    bHasPendingPermissionSettings = false;
    bWaitingForNotificationSettings = false;
    bNotificationTransitionInProgress = false;

    PendingRequestedSettings = FEVPopUpSettingsInfo();
    PendingPermissionSettings = FEVPopUpSettingsInfo();
}

bool AEVAppPlayerController::HasActiveNotificationSchedule() const
{
    return CurrentAcceptedSettings.PopUpIntervals != EEVPopUpIntervals::TurnedOff;
}

bool AEVAppPlayerController::HasNotificationIntervalChanged() const
{
    return bHasPendingRequestedSettings &&
           PendingRequestedSettings.PopUpIntervals != CurrentAcceptedSettings.PopUpIntervals;
}

bool AEVAppPlayerController::HasNotificationModeChanged() const
{
    return bHasPendingRequestedSettings &&
           PendingRequestedSettings.NotificationMode != CurrentAcceptedSettings.NotificationMode;
}

void AEVAppPlayerController::HandleNotificationPermissionResult(bool bGranted)
{
    if (!bHasPendingPermissionSettings)
    {
        UE_LOG(LogTemp, Warning,
               TEXT("Notification permission result received without pending notification settings."));
        return;
    }

    if (!EVGameInstance)
    {
        RejectPendingNotificationRequest();
        return;
    }

    if (bGranted && EVGameInstance->AreNotificationsEnabled())
    {
        CommitPendingPermissionSettings();
        return;
    }

    RejectPendingNotificationRequest();
}

void AEVAppPlayerController::HandleApplicationEnteredForeground()
{
    SynchronizeNotificationSettingsFromDevice();
    HandlePendingNotificationWord();

    if (!bWaitingForNotificationSettings || !bHasPendingPermissionSettings)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Returned from Android notification settings."));

    bWaitingForNotificationSettings = false;

    if (EVGameInstance && EVGameInstance->AreNotificationsEnabled())
    {
        CommitPendingPermissionSettings();
        return;
    }

    RejectPendingNotificationRequest();
}

void AEVAppPlayerController::PollNotificationState()
{
    if (bNotificationTransitionInProgress)
    {
        return;
    }

    SynchronizeNotificationSettingsFromDevice();
    HandlePendingNotificationWord();
}

void AEVAppPlayerController::HandlePendingNotificationWord()
{
    if (!EVGameInstance || !WidgetCommonEvents)
    {
        return;
    }

    if (CurrentAcceptedSettings.NotificationMode != EEVNotificationMode::RandomWord)
    {
        return;
    }

    FString NotificationWord;

    if (!EVGameInstance->ConsumePendingNotificationWord(NotificationWord) || NotificationWord.IsEmpty())
    {
        return;
    }

    WidgetCommonEvents->HandleOpenReviewWordsForNotification(NotificationWord);
}

void AEVAppPlayerController::SynchronizeNotificationSettingsFromDevice()
{
    if (!EVGameInstance || !WidgetCommonEvents)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot synchronize notification settings: controller is not initialized."));
        return;
    }

    FEVPopUpSettingsInfo StoredSettings;
    if (!EVGameInstance->GetStoredNotificationSettings(StoredSettings))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to read stored notification settings from the device."));
        return;
    }

    if (StoredSettings.PopUpIntervals == CurrentAcceptedSettings.PopUpIntervals &&
        StoredSettings.NotificationMode == CurrentAcceptedSettings.NotificationMode)
    {
        return;
    }

    CurrentAcceptedSettings = StoredSettings;
    ApplyResolvedNotificationSettings(CurrentAcceptedSettings);
}

void AEVAppPlayerController::HandleVocabularyValueActionRequested(const FEVVocabularyValueActionRequest& Request)
{
    PendingVocabularyValueActionRequest = Request;

    if (Request.ActionType == EEVVocabularyValueActionType::Synonym ||
        Request.ActionType == EEVVocabularyValueActionType::Antonym)
    {
        HandleRelationValueAction(Request);
        return;
    }

    if (Request.ActionType == EEVVocabularyValueActionType::Translation)
    {
        HandleTranslationValueAction(Request);
    }
}

void AEVAppPlayerController::HandleVocabularyLanguagePreferencesChanged(
    const FEVVocabularyLanguagePreferences& Preferences)
{
    if (!EVGameInstance)
    {
        return;
    }

    if (!EVGameInstance->SetVocabularyLanguagePreferences(Preferences))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to apply vocabulary language preferences."));
        return;
    }

    if (WidgetCommonEvents)
    {
        WidgetCommonEvents->HandleVocabularyLanguagePreferencesApplied(
            EVGameInstance->GetVocabularyLanguagePreferences());
    }
}

void AEVAppPlayerController::HandleRelationValueAction(const FEVVocabularyValueActionRequest& Request)
{
    if (!EVGameInstance)
    {
        return;
    }

    FString LookupWord = Request.Relation.NormalizedRelatedWord.TrimStartAndEnd();
    if (LookupWord.IsEmpty())
    {
        LookupWord = Request.Relation.RelatedWord.TrimStartAndEnd();
        LookupWord.ToLowerInline();
    }

    FVocabularyEntry ExistingEntry;
    const bool bExists = EVGameInstance->GetVocabularyEntryByWord(LookupWord, ExistingEntry);
    HandleCreateConfirmationDialog(bExists ? EEVConfirmationDialogType::ReviewExistingRelatedWord
                                           : EEVConfirmationDialogType::AddMissingRelatedWord,
                                   EEVWordEntryActionType::Unknown);
}

void AEVAppPlayerController::HandleTranslationValueAction(const FEVVocabularyValueActionRequest& Request)
{
    const EEVVocabularyLanguageSupportState SupportState =
        ResolveTranslationLanguageSupport(Request.Translation.TargetLanguage);

    if (SupportState == EEVVocabularyLanguageSupportState::Unsupported ||
        SupportState == EEVVocabularyLanguageSupportState::Unknown)
    {
        HandleCreateConfirmationDialog(EEVConfirmationDialogType::UnsupportedTranslationLanguage,
                                       EEVWordEntryActionType::Unknown);
        return;
    }

    if (SupportState == EEVVocabularyLanguageSupportState::SupportedWithoutContext)
    {
        HandleCreateConfirmationDialog(EEVConfirmationDialogType::CreateTranslationLanguageContext,
                                       EEVWordEntryActionType::Unknown);
        return;
    }

    HandleCreateConfirmationDialog(DoesTranslationWordExistInTargetContext(Request.Translation)
                                       ? EEVConfirmationDialogType::ReviewExistingTranslationWord
                                       : EEVConfirmationDialogType::AddMissingTranslationWord,
                                   EEVWordEntryActionType::Unknown);
}

bool AEVAppPlayerController::DoesTranslationWordExistInTargetContext(const FEVVocabularyTranslation& Translation) const
{
    if (!EVGameInstance)
    {
        return false;
    }

    EEVVocabularyTranslationLanguage TranslationLanguage = EEVVocabularyTranslationLanguage::None;
    EEVVocabularyDBContext TargetContext = EEVVocabularyDBContext::None;
    if (!EVVocabularyLanguage::TryParseTranslationLanguage(Translation.TargetLanguage, TranslationLanguage) ||
        !EVVocabularyLanguage::TryResolveDatabaseContextForTranslation(TranslationLanguage, TargetContext) ||
        TargetContext != EVGameInstance->GetVocabularyLanguagePreferences().DatabaseContext)
    {
        return false;
    }

    FVocabularyEntry ExistingEntry;
    return EVGameInstance->GetVocabularyEntryByWord(Translation.TranslationText, ExistingEntry);
}

EEVVocabularyLanguageSupportState
AEVAppPlayerController::ResolveTranslationLanguageSupport(const FString& LanguageCode) const
{
    EEVVocabularyTranslationLanguage TranslationLanguage = EEVVocabularyTranslationLanguage::None;
    if (!EVVocabularyLanguage::TryParseTranslationLanguage(LanguageCode, TranslationLanguage))
    {
        return EEVVocabularyLanguageSupportState::Unsupported;
    }

    EEVVocabularyDBContext TargetContext = EEVVocabularyDBContext::None;
    return EVVocabularyLanguage::TryResolveDatabaseContextForTranslation(TranslationLanguage, TargetContext)
               ? EEVVocabularyLanguageSupportState::SupportedWithContext
               : EEVVocabularyLanguageSupportState::SupportedWithoutContext;
}

void AEVAppPlayerController::HandleTranslationContextCreationConfirmed()
{
    // Future language-context creation entry point. Intentionally no action yet.
}

void AEVAppPlayerController::HandleTranslationExistingWordConfirmed()
{
    // Future cross-context Review Words entry point. Intentionally no action yet.
}

void AEVAppPlayerController::HandleTranslationMissingWordConfirmed()
{
    // Future cross-context Add Word entry point. Intentionally no action yet.
}

void AEVAppPlayerController::HandleVocabularyFiltersRequested()
{
    if (VocabularyFilterWidgetInstance)
    {
        return;
    }

    if (!VocabularyFilterWidgetClass || !EVGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot open vocabulary filters: class or game instance is missing."));
        return;
    }

    VocabularyFilterWidgetInstance = CreateWidget<UUserWidget>(this, VocabularyFilterWidgetClass);
    if (!VocabularyFilterWidgetInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create VocabularyFilterWidgetClass."));
        return;
    }

    VocabularyFilterWidgetProvider = Cast<IEVVocabularyFilterWidgetProvider>(VocabularyFilterWidgetInstance);
    if (!VocabularyFilterWidgetProvider)
    {
        UE_LOG(LogTemp, Error, TEXT("Vocabulary filter widget does not implement its provider interface."));
        VocabularyFilterWidgetInstance = nullptr;
        return;
    }

    VocabularyFilterWidgetProvider->SetInitialCriteria(EVGameInstance->GetActiveVocabularyQueryCriteria());
    VocabularyFilterWidgetProvider->GetFiltersAppliedEvent().AddUniqueDynamic(
        this, &ThisClass::HandleVocabularyFiltersApplied);
    VocabularyFilterWidgetProvider->GetCloseRequestedEvent().AddUniqueDynamic(
        this, &ThisClass::HandleVocabularyFilterWidgetCloseRequested);
    VocabularyFilterWidgetInstance->AddToViewport(9000);
}

void AEVAppPlayerController::HandleVocabularyFiltersApplied(const FEVVocabularyQueryCriteria& Criteria)
{
    if (EVGameInstance)
    {
        EVGameInstance->SetActiveVocabularyQueryCriteria(Criteria);
    }

    if (WidgetCommonEvents)
    {
        WidgetCommonEvents->HandleVocabularyFiltersApplied(Criteria);
    }

    HandleVocabularyFilterWidgetCloseRequested();
}

void AEVAppPlayerController::HandleVocabularyFilterWidgetCloseRequested()
{
    VocabularyFilterWidgetProvider = nullptr;
    DestroyWidget(VocabularyFilterWidgetInstance);
}
