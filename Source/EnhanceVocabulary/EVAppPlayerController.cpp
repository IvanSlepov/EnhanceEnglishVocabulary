// Fill out your copyright notice in the Description page of Project Settings.

#include "EVAppPlayerController.h"

#include "EVErrorProvider.h"
#include "EVGameInstance.h"
#include "EVErrorDisplayWidget.h"
#include "EVDisplayStatusProvider.h"
#include "EVFileExchangeWorkflowCoordinator.h"
#include "EVNotificationWorkflowCoordinator.h"
#include "EVVocabularyInteractionCoordinator.h"
#include "Kismet/KismetSystemLibrary.h"
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
    InitializeWorkflowCoordinators();
    if (EVGameInstance)
    {
        EVGameInstance->OnConnectionStateChanged.AddUniqueDynamic(this, &ThisClass::HandleConnectionStateChanged);
        HandleConnectionStateChanged(EVGameInstance->GetConnectionState());
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
        EVGameInstance->OnConnectionStateChanged.RemoveAll(this);
        EVGameInstance->OnNotificationPermissionResult().RemoveAll(this);

        EVGameInstance->OnFileOperationCompleted().RemoveAll(this);

        EVGameInstance->OnImportFilePickCompleted().RemoveAll(this);
    }

    if (ApplicationEnteredForegroundHandle.IsValid())
    {
        FCoreDelegates::ApplicationHasEnteredForegroundDelegate.Remove(ApplicationEnteredForegroundHandle);
    }

    if (FileExchangeWorkflowCoordinator)
    {
        FileExchangeWorkflowCoordinator->OnConfirmationRequested().RemoveAll(this);
        FileExchangeWorkflowCoordinator->OnLoadingChanged().RemoveAll(this);
        FileExchangeWorkflowCoordinator->OnStatusRequested().RemoveAll(this);
    }
    if (NotificationWorkflowCoordinator)
    {
        NotificationWorkflowCoordinator->OnConfirmationRequested().RemoveAll(this);
        NotificationWorkflowCoordinator->OnStatusRequested().RemoveAll(this);
    }
    if (VocabularyInteractionCoordinator)
    {
        VocabularyInteractionCoordinator->OnConfirmationRequested().RemoveAll(this);
        VocabularyInteractionCoordinator->OnLoadingChanged().RemoveAll(this);
        VocabularyInteractionCoordinator->OnStatusRequested().RemoveAll(this);
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
    EntryDetailsApplicationPort = Cast<IEVEntryDetailsApplicationPort>(RootWidgetInstance);
    VocabularyFilterApplicationPort = Cast<IEVVocabularyFilterApplicationPort>(RootWidgetInstance);
    GlobalPresentationApplicationPort = Cast<IEVGlobalPresentationApplicationPort>(RootWidgetInstance);
    FileExchangeApplicationPort = Cast<IEVFileExchangeApplicationPort>(RootWidgetInstance);
    NotificationSettingsApplicationPort = Cast<IEVNotificationSettingsApplicationPort>(RootWidgetInstance);
    VocabularyValueApplicationPort = Cast<IEVVocabularyValueApplicationPort>(RootWidgetInstance);
    ApplicationLifecyclePort = Cast<IEVApplicationLifecyclePort>(RootWidgetInstance);
    FeatureNavigationApplicationPort = Cast<IEVFeatureNavigationApplicationPort>(RootWidgetInstance);

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

    if (EntryDetailsApplicationPort)
    {
        EntryDetailsApplicationPort->GetEntryDetailsRequestedEvent().AddUObject(
            this, &ThisClass::HandleEntryDetailsRequested);
        EntryDetailsApplicationPort->GetEntryDetailsCloseRequestedEvent().AddUObject(
            this, &ThisClass::HandleEntryDetailsCloseRequested);
        EntryDetailsApplicationPort->GetEntryDetailsDeleteRequestedEvent().AddUObject(
            this, &ThisClass::HandleEntryDetailsDeleteRequested);
        EntryDetailsApplicationPort->GetEntryDetailsSaveRequestedEvent().AddUObject(
            this, &ThisClass::HandleEntryDetailsSaveRequested);
    }

    if (VocabularyFilterApplicationPort)
    {
        VocabularyFilterApplicationPort->GetVocabularyFilterPresentationRequestedEvent().AddUObject(
            this, &ThisClass::HandleVocabularyFiltersRequested);
        VocabularyFilterApplicationPort->GetVocabularyFilterCriteriaChangeRequestedEvent().AddUObject(
            this, &ThisClass::HandleVocabularyFiltersApplied);
    }

    if (GlobalPresentationApplicationPort)
    {
        GlobalPresentationApplicationPort->GetGlobalErrorRequestedEvent().AddUObject(this,
                                                                                     &ThisClass::HandleWidgetErrors);
        GlobalPresentationApplicationPort->GetGlobalLoadingStateChangedEvent().AddUObject(
            this, &ThisClass::HandleLoadingSpinner);
        GlobalPresentationApplicationPort->GetGlobalStatusRequestedEvent().AddUObject(
            this, &ThisClass::HandleActionStatusWidget);
    }

    if (FileExchangeApplicationPort)
    {
        FileExchangeApplicationPort->GetFileOperationRequestedEvent().AddUObject(this,
                                                                                 &ThisClass::HandleIssuedFileOperation);
    }

    if (NotificationSettingsApplicationPort)
    {
        NotificationSettingsApplicationPort->GetNotificationSettingsChangeRequestedEvent().AddUObject(
            this, &ThisClass::HandleNotificationSettingsChanged);
    }

    if (VocabularyValueApplicationPort)
    {
        VocabularyValueApplicationPort->GetVocabularyValueApplicationRequestedEvent().AddUObject(
            this, &ThisClass::HandleVocabularyValueActionRequested);
    }

    if (ApplicationLifecyclePort)
    {
        ApplicationLifecyclePort->GetApplicationExitRequestedEvent().AddUObject(
            this, &ThisClass::HandleApplicationExitRequested);
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

        if (VocabularyPreferencesApplicationPort)
        {
            FEVVocabularyPreferencesState InitialPreferencesState;
            InitialPreferencesState.Preferences = EVGameInstance->GetVocabularyLanguagePreferences();
            VocabularyPreferencesApplicationPort->ApplyVocabularyPreferencesState(InitialPreferencesState);
        }
    }

    UpdateWorkflowCoordinatorPorts();
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

    if (EntryDetailsApplicationPort)
    {
        EntryDetailsApplicationPort->GetEntryDetailsRequestedEvent().RemoveAll(this);
        EntryDetailsApplicationPort->GetEntryDetailsCloseRequestedEvent().RemoveAll(this);
        EntryDetailsApplicationPort->GetEntryDetailsDeleteRequestedEvent().RemoveAll(this);
        EntryDetailsApplicationPort->GetEntryDetailsSaveRequestedEvent().RemoveAll(this);
    }

    if (VocabularyFilterApplicationPort)
    {
        VocabularyFilterApplicationPort->GetVocabularyFilterPresentationRequestedEvent().RemoveAll(this);
        VocabularyFilterApplicationPort->GetVocabularyFilterCriteriaChangeRequestedEvent().RemoveAll(this);
    }

    if (GlobalPresentationApplicationPort)
    {
        GlobalPresentationApplicationPort->GetGlobalErrorRequestedEvent().RemoveAll(this);
        GlobalPresentationApplicationPort->GetGlobalLoadingStateChangedEvent().RemoveAll(this);
        GlobalPresentationApplicationPort->GetGlobalStatusRequestedEvent().RemoveAll(this);
    }

    if (FileExchangeApplicationPort)
    {
        FileExchangeApplicationPort->GetFileOperationRequestedEvent().RemoveAll(this);
    }

    if (NotificationSettingsApplicationPort)
    {
        NotificationSettingsApplicationPort->GetNotificationSettingsChangeRequestedEvent().RemoveAll(this);
    }

    if (VocabularyValueApplicationPort)
    {
        VocabularyValueApplicationPort->GetVocabularyValueApplicationRequestedEvent().RemoveAll(this);
    }

    if (ApplicationLifecyclePort)
    {
        ApplicationLifecyclePort->GetApplicationExitRequestedEvent().RemoveAll(this);
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
    EntryDetailsApplicationPort = nullptr;
    VocabularyFilterApplicationPort = nullptr;
    GlobalPresentationApplicationPort = nullptr;
    FileExchangeApplicationPort = nullptr;
    NotificationSettingsApplicationPort = nullptr;
    VocabularyValueApplicationPort = nullptr;
    ApplicationLifecyclePort = nullptr;
    FeatureNavigationApplicationPort = nullptr;
    UpdateWorkflowCoordinatorPorts();
}

void AEVAppPlayerController::InitializeWorkflowCoordinators()
{
    FileExchangeWorkflowCoordinator = NewObject<UEVFileExchangeWorkflowCoordinator>(this);
    NotificationWorkflowCoordinator = NewObject<UEVNotificationWorkflowCoordinator>(this);
    VocabularyInteractionCoordinator = NewObject<UEVVocabularyInteractionCoordinator>(this);

    if (FileExchangeWorkflowCoordinator)
    {
        FileExchangeWorkflowCoordinator->Initialize(EVGameInstance, VocabularyLibraryApplicationPort,
                                                    WidgetCommonEvents);
        FileExchangeWorkflowCoordinator->OnConfirmationRequested().AddUObject(
            this, &ThisClass::HandleCreateConfirmationDialog);
        FileExchangeWorkflowCoordinator->OnLoadingChanged().AddUObject(this, &ThisClass::HandleLoadingSpinner);
        FileExchangeWorkflowCoordinator->OnStatusRequested().AddUObject(this, &ThisClass::HandleActionStatusWidget);
    }

    if (NotificationWorkflowCoordinator)
    {
        NotificationWorkflowCoordinator->Initialize(EVGameInstance, NotificationSettingsApplicationPort,
                                                    FeatureNavigationApplicationPort, WidgetCommonEvents);
        NotificationWorkflowCoordinator->OnConfirmationRequested().AddUObject(
            this, &ThisClass::HandleCreateConfirmationDialog);
        NotificationWorkflowCoordinator->OnStatusRequested().AddUObject(this, &ThisClass::HandleActionStatusWidget);
    }

    if (VocabularyInteractionCoordinator)
    {
        VocabularyInteractionCoordinator->Initialize(EVGameInstance, EntryDetailsApplicationPort,
                                                     VocabularyFilterApplicationPort, FeatureNavigationApplicationPort,
                                                     VocabularyLibraryApplicationPort, WidgetCommonEvents);
        VocabularyInteractionCoordinator->OnConfirmationRequested().AddUObject(
            this, &ThisClass::HandleCreateConfirmationDialog);
        VocabularyInteractionCoordinator->OnLoadingChanged().AddUObject(this, &ThisClass::HandleLoadingSpinner);
        VocabularyInteractionCoordinator->OnStatusRequested().AddUObject(this, &ThisClass::HandleActionStatusWidget);
    }
}

void AEVAppPlayerController::UpdateWorkflowCoordinatorPorts()
{
    if (FileExchangeWorkflowCoordinator)
    {
        FileExchangeWorkflowCoordinator->UpdatePresentationPorts(VocabularyLibraryApplicationPort, WidgetCommonEvents);
    }
    if (NotificationWorkflowCoordinator)
    {
        NotificationWorkflowCoordinator->UpdatePresentationPorts(NotificationSettingsApplicationPort,
                                                                 FeatureNavigationApplicationPort, WidgetCommonEvents);
    }
    if (VocabularyInteractionCoordinator)
    {
        VocabularyInteractionCoordinator->UpdatePresentationPorts(
            EntryDetailsApplicationPort, VocabularyFilterApplicationPort, FeatureNavigationApplicationPort,
            VocabularyLibraryApplicationPort, WidgetCommonEvents);
    }
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

void AEVAppPlayerController::HandleEntryDetailsRequested(const FEVVocabularyRecord& Record)
{
    if (VocabularyInteractionCoordinator)
    {
        VocabularyInteractionCoordinator->PresentEntryDetails(Record);
    }
}

void AEVAppPlayerController::HandleConnectionStateChanged(const EEVConnectionState State)
{
    if (!NetworkConnectivityApplicationPort)
    {
        return;
    }

    switch (State)
    {
    case EEVConnectionState::Online:
        NetworkConnectivityApplicationPort->ApplyNetworkConnectivityState(EEVApplicationConnectivityState::Online);
        break;
    case EEVConnectionState::Connecting:
        NetworkConnectivityApplicationPort->ApplyNetworkConnectivityState(EEVApplicationConnectivityState::Connecting);
        break;
    default:
        NetworkConnectivityApplicationPort->ApplyNetworkConnectivityState(EEVApplicationConnectivityState::Offline);
        break;
    }
}

void AEVAppPlayerController::HandleApplicationExitRequested()
{
    UKismetSystemLibrary::QuitGame(GetWorld(), this, EQuitPreference::Quit, false);
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
    if (GlobalPresentationResolutionPort)
    {
        GlobalPresentationResolutionPort->ApplyGlobalErrorResolution(EVErrorInfo);
    }
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
    if (VocabularyInteractionCoordinator)
    {
        VocabularyInteractionCoordinator->PresentLegacyEntryDetails(CurrentWordEntryWidgetInfo);
    }
}

// Will try to control the action that came from widget (ImportExportDB in this case)
// from the PC rather than on the widget's side. If this approach proves to be cleaner than
// what we have now we'll try to refactor other parts of the app to implement it.
void AEVAppPlayerController::HandleIssuedFileOperation(const FEVFileOperationInfo& IssuedFileOperation)
{
    if (FileExchangeWorkflowCoordinator)
    {
        FileExchangeWorkflowCoordinator->RequestOperation(IssuedFileOperation);
    }
}

void AEVAppPlayerController::HandleFileOperationCompleted(const FEVRequestedActionInfo& RequestedActionInfo)
{
    if (FileExchangeWorkflowCoordinator)
    {
        FileExchangeWorkflowCoordinator->HandleOperationCompleted(RequestedActionInfo);
    }
}

void AEVAppPlayerController::HandleImportFilePickCompleted(const FEVFileExchangeResultInfo& ResultInfo)
{
    if (FileExchangeWorkflowCoordinator)
    {
        FileExchangeWorkflowCoordinator->HandleImportFilePickCompleted(ResultInfo);
    }
}

void AEVAppPlayerController::HandleEntryDetailsCloseRequested()
{
    if (VocabularyInteractionCoordinator)
    {
        VocabularyInteractionCoordinator->RequestEntryDetailsClose();
    }
}

void AEVAppPlayerController::HandleEntryDetailsSaveRequested(const FEVVocabularyRecord& NewVocabularyRecord)
{
    if (VocabularyInteractionCoordinator)
    {
        VocabularyInteractionCoordinator->RequestEntryDetailsSave(NewVocabularyRecord);
    }
}

void AEVAppPlayerController::HandleEntryDetailsDeleteRequested()
{
    if (VocabularyInteractionCoordinator)
    {
        VocabularyInteractionCoordinator->RequestEntryDetailsDelete();
    }
}

void AEVAppPlayerController::HandleCreateConfirmationDialog(EEVConfirmationDialogType DialogType,
                                                            EEVWordEntryActionType PendingActionType)
{
    PendingConfirmationDialogType = DialogType;
    (void)PendingActionType;

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
    if (VocabularyInteractionCoordinator)
    {
        ConfirmationDialogInfo.SubjectValue = VocabularyInteractionCoordinator->GetConfirmationSubject(DialogType);
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

    if (FileExchangeWorkflowCoordinator &&
        FileExchangeWorkflowCoordinator->ResolveConfirmation(ConfirmedDialogType, bIsOperationConfirmed))
    {
        return;
    }
    if (NotificationWorkflowCoordinator &&
        NotificationWorkflowCoordinator->ResolveConfirmation(ConfirmedDialogType, bIsOperationConfirmed))
    {
        return;
    }
    if (VocabularyInteractionCoordinator &&
        VocabularyInteractionCoordinator->ResolveConfirmation(ConfirmedDialogType, bIsOperationConfirmed))
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("No workflow coordinator accepted confirmation dialog type %d."),
           static_cast<int32>(ConfirmedDialogType));
}

void AEVAppPlayerController::HandleNotificationSettingsChanged(const FEVPopUpSettingsInfo& RequestedSettings)
{
    if (NotificationWorkflowCoordinator)
    {
        NotificationWorkflowCoordinator->RequestSettingsChange(RequestedSettings);
    }
}

void AEVAppPlayerController::HandleNotificationPermissionResult(const bool bGranted)
{
    if (NotificationWorkflowCoordinator)
    {
        NotificationWorkflowCoordinator->HandlePermissionResult(bGranted);
    }
}

void AEVAppPlayerController::HandleApplicationEnteredForeground()
{
    if (NotificationWorkflowCoordinator)
    {
        NotificationWorkflowCoordinator->HandleApplicationEnteredForeground();
    }
}

void AEVAppPlayerController::PollNotificationState()
{
    if (NotificationWorkflowCoordinator)
    {
        NotificationWorkflowCoordinator->PollDeviceState();
    }
}

void AEVAppPlayerController::HandlePendingNotificationWord()
{
    if (NotificationWorkflowCoordinator)
    {
        NotificationWorkflowCoordinator->HandlePendingNotificationWord();
    }
}

void AEVAppPlayerController::SynchronizeNotificationSettingsFromDevice()
{
    if (NotificationWorkflowCoordinator)
    {
        NotificationWorkflowCoordinator->SynchronizeFromDevice();
    }
}

void AEVAppPlayerController::HandleVocabularyValueActionRequested(const FEVVocabularyValueActionRequest& Request)
{
    if (VocabularyInteractionCoordinator)
    {
        VocabularyInteractionCoordinator->RequestVocabularyValueAction(Request);
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

void AEVAppPlayerController::HandleVocabularyFiltersRequested()
{
    if (VocabularyInteractionCoordinator)
    {
        VocabularyInteractionCoordinator->RequestVocabularyFilters();
    }
}

void AEVAppPlayerController::HandleVocabularyFiltersApplied(const FEVVocabularyQueryCriteria& Criteria)
{
    if (VocabularyInteractionCoordinator)
    {
        VocabularyInteractionCoordinator->ApplyVocabularyFilters(Criteria);
    }
}
