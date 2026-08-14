// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "EVRequestedActionTypes.h"
#include "EVErrorTypes.h"
#include "EVWordEntryActionTypes.h"
#include "EVConfirmationDialogActionTypes.h"
#include "EVConfirmationDialogWidgetProvider.h"

#include "EVGameInstance.h"
#include "EVWidgetCommonEvents.h"
#include "EVFileExchangeTypes.h"
#include "EVPopUpSettingsTypes.h"
#include "EVVocabularyInteractionTypes.h"
#include "EVVocabularyFilterTypes.h"

#include "EVVocabularyLanguageTypes.h"
#include "EVVocabularySearchApplicationPort.h"
#include "EVVocabularyLibraryApplicationPort.h"
#include "EVVocabularyPreferencesApplicationPort.h"
#include "EVNetworkConnectivityApplicationPort.h"
#include "EVGlobalPresentationResolutionPort.h"
#include "EVEntryDetailsApplicationPort.h"
#include "EVVocabularyFilterApplicationPort.h"
#include "EVGlobalPresentationApplicationPort.h"
#include "EVFileExchangeApplicationPort.h"
#include "EVNotificationSettingsApplicationPort.h"
#include "EVVocabularyValueApplicationPort.h"
#include "EVApplicationLifecyclePort.h"
#include "EVFeatureNavigationApplicationPort.h"
#include "EVAppPlayerController.generated.h"

class UEVFileExchangeWorkflowCoordinator;
class UEVNotificationWorkflowCoordinator;
class UEVVocabularyInteractionCoordinator;

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWidgetsErrorResolved, const FEVErrorInfo&, ErrorInfo);

UCLASS()
class ENHANCEVOCABULARY_API AEVAppPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AEVAppPlayerController();

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> RootWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> RootWidgetInstance;

    IEVWidgetCommonEvents* WidgetCommonEvents = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> ErrorWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> ErrorWidgetInstance;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> LoadingSpinnerClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> LoadingSpinnerInstance;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> RequestedActionStatusWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> RequestedActionStatusWidgetInstance;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> ConfirmationDialogWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> ConfirmationDialogWidgetInstance;

    IEVConfirmationDialogWidgetProvider* ConfirmationDialogWidget = nullptr;

    class UEVGameInstance* EVGameInstance;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "PC Events")
    FOnWidgetsErrorResolved OnWidgetsErrorResolved;

    void HandleFileOperationCompleted(const FEVRequestedActionInfo& RequestedActionInfo);
    void HandleImportFilePickCompleted(const FEVFileExchangeResultInfo& ResultInfo);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void InitEVAppPlayerController();

private:
    void HandleApplicationEnteredForeground();
    void PollNotificationState();
    void SynchronizeNotificationSettingsFromDevice();
    void HandlePendingNotificationWord();
    void BindApplicationPorts();
    void UnbindApplicationPorts();
    void InitializeWorkflowCoordinators();
    void UpdateWorkflowCoordinatorPorts();

    void HandleVocabularySearchRequested(const FEVVocabularySearchRequest& Request);
    void HandleVocabularyRecordRequested(const FEVVocabularyRecordRequest& Request);
    void HandleVocabularyQueryRequested(const FEVVocabularyQueryRequest& Request);
    void HandleVocabularyMutationRequested(const FEVVocabularyMutationRequest& Request);
    void HandleVocabularyPreferencesChangeRequested(const FEVVocabularyPreferencesChangeRequest& Request);

    void HandleVocabularySearchOutcomeReady(const FEVVocabularySearchOutcome& Outcome);
    void HandleVocabularyRecordOutcomeReady(const FEVVocabularyRecordOutcome& Outcome);
    void HandleVocabularyQueryOutcomeReady(const FEVVocabularyQueryOutcome& Outcome);
    void HandleVocabularyMutationOutcomeReady(const FEVVocabularyMutationOutcome& Outcome);
    void HandleVocabularyChanged(const FEVVocabularyChangeInfo& ChangeInfo);
    void HandleVocabularyPreferencesStateReady(const FEVVocabularyPreferencesState& State);
    void HandleEntryDetailsRequested(const FEVVocabularyRecord& Record);
    void HandleApplicationExitRequested();

    UFUNCTION()
    void HandleConnectionStateChanged(EEVConnectionState State);

    IEVVocabularySearchApplicationPort* VocabularySearchApplicationPort = nullptr;
    IEVVocabularyLibraryApplicationPort* VocabularyLibraryApplicationPort = nullptr;
    IEVVocabularyPreferencesApplicationPort* VocabularyPreferencesApplicationPort = nullptr;
    IEVNetworkConnectivityApplicationPort* NetworkConnectivityApplicationPort = nullptr;
    IEVGlobalPresentationResolutionPort* GlobalPresentationResolutionPort = nullptr;
    IEVEntryDetailsApplicationPort* EntryDetailsApplicationPort = nullptr;
    IEVVocabularyFilterApplicationPort* VocabularyFilterApplicationPort = nullptr;
    IEVGlobalPresentationApplicationPort* GlobalPresentationApplicationPort = nullptr;
    IEVFileExchangeApplicationPort* FileExchangeApplicationPort = nullptr;
    IEVNotificationSettingsApplicationPort* NotificationSettingsApplicationPort = nullptr;
    IEVVocabularyValueApplicationPort* VocabularyValueApplicationPort = nullptr;
    IEVApplicationLifecyclePort* ApplicationLifecyclePort = nullptr;
    IEVFeatureNavigationApplicationPort* FeatureNavigationApplicationPort = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UEVFileExchangeWorkflowCoordinator> FileExchangeWorkflowCoordinator;

    UPROPERTY(Transient)
    TObjectPtr<UEVNotificationWorkflowCoordinator> NotificationWorkflowCoordinator;

    UPROPERTY(Transient)
    TObjectPtr<UEVVocabularyInteractionCoordinator> VocabularyInteractionCoordinator;

    FEVErrorInfo EVErrorInfo;

    FDelegateHandle ApplicationEnteredForegroundHandle;
    FTimerHandle NotificationStatePollTimerHandle;
    EEVConfirmationDialogType PendingConfirmationDialogType = EEVConfirmationDialogType::Unknown;

    UFUNCTION()
    void HandleWidgetErrors(const FEVErrorInfo& WidgetErrorInfo);

    UFUNCTION()
    void HandleErrorWidgetDestroyed();

    UFUNCTION()
    void HandleLoadingSpinner(bool bDisplayLoadingSpinner);

    UFUNCTION()
    void HandleActionStatusWidget(const FEVRequestedActionInfo& RequestedActionInfo);

    UFUNCTION()
    void HandleWordEntryWidget(const FEVWordEntryActionInfo& CurrentWordEntryWidgetInfo);

    UFUNCTION()
    void HandleIssuedFileOperation(const FEVFileOperationInfo& IssuedFileOperation);

    UFUNCTION()
    void HandleVocabularyValueActionRequested(const FEVVocabularyValueActionRequest& Request);

    UFUNCTION()
    void HandleVocabularyFiltersRequested();

    UFUNCTION()
    void HandleVocabularyFiltersApplied(const FEVVocabularyQueryCriteria& Criteria);

    UFUNCTION()
    void HandleVocabularyLanguagePreferencesChanged(const FEVVocabularyLanguagePreferences& Preferences);

    void HandleEntryDetailsCloseRequested();
    void HandleEntryDetailsDeleteRequested();
    void HandleEntryDetailsSaveRequested(const FEVVocabularyRecord& NewVocabularyRecord);

    // Handlers for the "EVConfirmationDialogWidget.h"
    UFUNCTION()
    void HandleCreateConfirmationDialog(EEVConfirmationDialogType DialogType, EEVWordEntryActionType PendingActionType);

    UFUNCTION()
    void HandleConfirmationDialog_ButtonPressed(bool bIsOperationConfirmed);

    // Handle complete notification settings emitted by the settings widget.
    UFUNCTION()
    void HandleNotificationSettingsChanged(const FEVPopUpSettingsInfo& RequestedSettings);

    void HandleNotificationPermissionResult(bool bGranted);
};
