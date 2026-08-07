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
#include "EVWordEntryDisplayWidgetProvider.h"
#include "EVGameInstance.h"
#include "EVWidgetCommonEvents.h"
#include "EVFileExchangeTypes.h"
#include "EVPopUpSettingsTypes.h"
#include "EVVocabularyInteractionTypes.h"
#include "EVVocabularyFilterTypes.h"
#include "EVVocabularyFilterWidgetProvider.h"
#include "EVVocabularyLanguageTypes.h"
#include "EVAppPlayerController.generated.h"

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
    TSubclassOf<UUserWidget> DetailedWordEntryWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> DetailedWordEntryWidgetInstance;

    IEVWordEntryDisplayWidgetProvider* DetailedWordEntryDisplay = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> ConfirmationDialogWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> ConfirmationDialogWidgetInstance;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> VocabularyFilterWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> VocabularyFilterWidgetInstance;

    IEVVocabularyFilterWidgetProvider* VocabularyFilterWidgetProvider = nullptr;

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

    FEVErrorInfo EVErrorInfo;

    FDelegateHandle ApplicationEnteredForegroundHandle;
    FTimerHandle NotificationStatePollTimerHandle;
    // Cache the data we receive from the the WordEntry
    // we decided to review
    FEVWordEntryActionInfo CachedWordEntryWidgetInfo;

    // Structured record currently confirmed and displayed in Detailed View.
    FEVVocabularyRecord CachedConfirmedVocabularyRecord;

    // Structured edits waiting for confirmation.
    FEVVocabularyRecord CachedPendingVocabularyRecord;

    FEVFileOperationInfo PendingFileOperationInfo;

    EEVConfirmationDialogType PendingConfirmationDialogType = EEVConfirmationDialogType::Unknown;
    FEVVocabularyValueActionRequest PendingVocabularyValueActionRequest;

    // ======== Notification settings and transitions ===========
    // Settings that are currently accepted by the controller.
    FEVPopUpSettingsInfo CurrentAcceptedSettings;

    // Complete settings snapshot most recently requested by the widget.
    FEVPopUpSettingsInfo PendingRequestedSettings;

    // Settings waiting specifically for Android permission/settings resolution.
    FEVPopUpSettingsInfo PendingPermissionSettings;

    bool bHasPendingRequestedSettings = false;
    bool bHasPendingPermissionSettings = false;
    bool bWaitingForNotificationSettings = false;
    bool bNotificationTransitionInProgress = false;
    //============================================================

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
    void HandleVocabularyFilterWidgetCloseRequested();

    UFUNCTION()
    void HandleVocabularyLanguagePreferencesChanged(const FEVVocabularyLanguagePreferences& Preferences);

    void HandleRelationValueAction(const FEVVocabularyValueActionRequest& Request);
    void HandleTranslationValueAction(const FEVVocabularyValueActionRequest& Request);
    EEVVocabularyLanguageSupportState ResolveTranslationLanguageSupport(const FString& LanguageCode) const;
    bool DoesTranslationWordExistInTargetContext(const FEVVocabularyTranslation& Translation) const;
    void HandleTranslationContextCreationConfirmed();
    void HandleTranslationExistingWordConfirmed();
    void HandleTranslationMissingWordConfirmed();

    // Handlers for the "EVWordEntryWidgetDetailed.h" buttons
    UFUNCTION()
    void HandleDetailedViewButtonPressed();

    UFUNCTION()
    void HandleDetailedEditButtonPressed();

    UFUNCTION()
    void HandleDetailedDeleteButtonPressed();

    UFUNCTION()
    void HandleDetailedSaveChangesButtonPressed(const FEVVocabularyRecord& NewVocabularyRecord);

    // Handlers for the "EVConfirmationDialogWidget.h"
    UFUNCTION()
    void HandleCreateConfirmationDialog(EEVConfirmationDialogType DialogType, EEVWordEntryActionType PendingActionType);

    UFUNCTION()
    void HandleConfirmationDialog_ButtonPressed(bool bIsOperationConfirmed);

    // Handle Edit word entry
    UFUNCTION()
    void ProcessConfirmedWordUpdate();

    // Handle Delete word entry
    UFUNCTION()
    void ProcessConfirmedWordDelete();

    // Handle widget destruction
    void DestroyWidget(TObjectPtr<UUserWidget>& Widget);

    // Handle complete notification settings emitted by the settings widget.
    UFUNCTION()
    void HandleNotificationSettingsChanged(const FEVPopUpSettingsInfo& RequestedSettings);

    void EvaluateNotificationSettingsChange();
    void EvaluateRandomWordModeChange();
    void EvaluateTestModeChange();
    void HandleNotificationIntervalChange();
    void RequestNotificationModeChange();
    void CommitPendingModeChange();
    void RejectPendingModeChange();
    void ProcessNotificationSettingsRequest(const FEVPopUpSettingsInfo& RequestedSettings);

    UFUNCTION()
    void ApplyResolvedNotificationSettings(const FEVPopUpSettingsInfo& ResolvedSettings);

    void CommitPendingPermissionSettings();
    void RejectPendingNotificationRequest();
    void ClearPendingNotificationRequest();

    bool HasActiveNotificationSchedule() const;
    bool HasNotificationIntervalChanged() const;
    bool HasNotificationModeChanged() const;

    void HandleNotificationPermissionResult(bool bGranted);
};
