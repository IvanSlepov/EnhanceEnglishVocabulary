// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Widgets/SWidget.h"
#include "Components/PanelWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/WidgetSwitcherSlot.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EVAddWordWidget.h"
#include "EVMainMenuWidget.h"
#include "EVNoMenuWidget.h"
#include "EVReviewWordsWidget.h"
#include "EVAppSettingsWidget.h"
#include "EVImportExportDBWidget.h"
#include "EVPopUpSettingsWidget.h"
#include "EVErrorProvider.h"
#include "EVErrorTypes.h"
#include "EVRequestedActionTypes.h"
#include "EVConnectionTypesAndEnums.h"
#include "EVWidgetCommonEvents.h"
#include "EVWordEntryActionTypes.h"
#include "EVFileExchangeTypes.h"
#include "EVPopUpSettingsTypes.h"
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
#include "EVFeatureRoles.h"
#include "EVFeatureRegistry.h"
#include "EVWordEntryDisplayWidgetProvider.h"
#include "EVVocabularyFilterWidgetProvider.h"

#include "EVRootWidget.generated.h"

/**
 *
 */

UCLASS()
class ENHANCEVOCABULARYUI_API UEVRootWidget : public UUserWidget,
                                              public IEVErrorProvider,
                                              public IEVWidgetCommonEvents,
                                              public IEVVocabularySearchApplicationPort,
                                              public IEVVocabularyLibraryApplicationPort,
                                              public IEVVocabularyPreferencesApplicationPort,
                                              public IEVNetworkConnectivityApplicationPort,
                                              public IEVGlobalPresentationResolutionPort,
                                              public IEVEntryDetailsApplicationPort,
                                              public IEVVocabularyFilterApplicationPort,
                                              public IEVGlobalPresentationApplicationPort,
                                              public IEVFileExchangeApplicationPort,
                                              public IEVNotificationSettingsApplicationPort,
                                              public IEVVocabularyValueApplicationPort,
                                              public IEVApplicationLifecyclePort,
                                              public IEVFeatureNavigationApplicationPort
{
    GENERATED_BODY()

public:
    // Native UE5.7 widgets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_Menu;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UImage> Image_ConnectionState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UWidgetSwitcher> WidgetSwitcher_Main;

    /*The EV app WBPs added to the WidgetSwitcher_Main*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Online Dependant")
    TObjectPtr<UEVAddWordWidget> AddWord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UEVNoMenuWidget> NoMenu;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UEVMainMenuWidget> MainMenu;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UEVReviewWordsWidget> ReviewWords;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UEVAppSettingsWidget> Settings_SelectWebProviders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UEVImportExportDBWidget> ImportExportDB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UEVPopUpSettingsWidget> PopUpSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UUserWidget> WordEntryWIdgetDetailedView;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UUserWidget> VocabularyFilterWidget;

    /*Events*/

    // Interface derrived event declaration
    virtual FOnEVError& GetOnErrorEvent() override
    {
        return OnRootWidgetError;
    }

    virtual FOnLoadingDataTriggerred* GetLoadingSpinnerEvent() override
    {
        return &OnLoadingDataTriggerred;
    }

    virtual FOnActionRequested* GetRequestedActionInfo() override
    {
        return &OnActionRequested;
    }

    virtual FOnWordEntryWidgetControlsActivated* GetCurrentWordEntryWidgetActionInfo() override
    {
        return &OnWordEntryWidgetControlsActivated;
    }

    virtual FOnImportExportDownloadDBOperationIssued* GetIssuedFileOperationInfo() override
    {
        return &OnImportExportDownloadDBOperationIssued;
    }

    virtual FOnPopUpIntervalSelectedFromSettings* GetSelectedPopUpInterval() override
    {
        return &OnPopUpIntervalSelectedFromSettings;
    }

    virtual FOnVocabularyValueActionRequested* GetVocabularyValueActionRequestedEvent() override
    {
        return &OnVocabularyValueActionRequested;
    }

    virtual FOnVocabularyFiltersRequested* GetVocabularyFiltersRequestedEvent() override
    {
        return &OnVocabularyFiltersRequested;
    }

    virtual FOnVocabularyLanguagePreferencesChangedFromWidgets* GetVocabularyLanguagePreferencesChangedEvent() override
    {
        return &OnVocabularyLanguagePreferencesChanged;
    }

    virtual void HandleWordEntryChanged(const FEVWordEntryActionInfo& WordEntryActionInfo) override;

    virtual void HandleReviewWordsRefresh() override;
    virtual void HandleOpenReviewWordsForNotification(const FString& Word) override;
    virtual void HandleOpenAddWordWithWord(const FString& Word) override;
    virtual void HandleVocabularyFiltersApplied(const FEVVocabularyQueryCriteria& Criteria) override;
    virtual void
    HandleVocabularyLanguagePreferencesApplied(const FEVVocabularyLanguagePreferences& Preferences) override;

    virtual FOnEVVocabularySearchRequested& GetVocabularySearchRequestedEvent() override
    {
        return OnVocabularySearchRequested;
    }

    virtual void ApplyVocabularySearchOutcome(const FEVVocabularySearchOutcome& Outcome) override;

    virtual FOnEVVocabularyRecordRequested& GetVocabularyRecordRequestedEvent() override
    {
        return OnVocabularyRecordRequested;
    }

    virtual FOnEVVocabularyQueryRequested& GetVocabularyQueryRequestedEvent() override
    {
        return OnVocabularyQueryRequested;
    }

    virtual FOnEVVocabularyMutationRequested& GetVocabularyMutationRequestedEvent() override
    {
        return OnVocabularyMutationRequested;
    }

    virtual void ApplyVocabularyRecordOutcome(const FEVVocabularyRecordOutcome& Outcome) override;
    virtual void ApplyVocabularyQueryOutcome(const FEVVocabularyQueryOutcome& Outcome) override;
    virtual void ApplyVocabularyMutationOutcome(const FEVVocabularyMutationOutcome& Outcome) override;
    virtual void ApplyVocabularyChanged(const FEVVocabularyChangeInfo& ChangeInfo) override;

    virtual FOnEVVocabularyPreferencesChangeRequested& GetVocabularyPreferencesChangeRequestedEvent() override
    {
        return OnVocabularyPreferencesChangeRequested;
    }

    virtual void ApplyVocabularyPreferencesState(const FEVVocabularyPreferencesState& State) override;
    virtual void ApplyNetworkConnectivityState(EEVApplicationConnectivityState State) override;
    virtual void ApplyGlobalErrorResolution(const FEVErrorInfo& ErrorInfo) override;

    virtual FOnEVEntryDetailsRequested& GetEntryDetailsRequestedEvent() override
    {
        return OnEntryDetailsRequested;
    }

    virtual FOnEVEntryDetailsCloseRequested& GetEntryDetailsCloseRequestedEvent() override
    {
        return OnEntryDetailsCloseRequested;
    }

    virtual FOnEVEntryDetailsDeleteRequested& GetEntryDetailsDeleteRequestedEvent() override
    {
        return OnEntryDetailsDeleteRequested;
    }

    virtual FOnEVEntryDetailsSaveRequested& GetEntryDetailsSaveRequestedEvent() override
    {
        return OnEntryDetailsSaveRequested;
    }

    virtual void PresentEntryDetails(const FEVVocabularyRecord& Record) override;
    virtual void DismissEntryDetails() override;

    virtual FOnEVVocabularyFiltersRequested& GetVocabularyFilterPresentationRequestedEvent() override
    {
        return OnVocabularyFiltersRequestedByFeature;
    }

    virtual FOnEVVocabularyFilterCriteriaChangeRequested& GetVocabularyFilterCriteriaChangeRequestedEvent() override
    {
        return OnVocabularyFilterCriteriaChangeRequested;
    }

    virtual void PresentVocabularyFilters(const FEVVocabularyQueryCriteria& Criteria) override;
    virtual void ApplyVocabularyFilterCriteria(const FEVVocabularyQueryCriteria& Criteria) override;

    virtual FOnEVGlobalErrorRequested& GetGlobalErrorRequestedEvent() override
    {
        return OnGlobalErrorRequested;
    }

    virtual FOnEVGlobalLoadingStateChanged& GetGlobalLoadingStateChangedEvent() override
    {
        return OnGlobalLoadingStateChanged;
    }

    virtual FOnEVGlobalStatusRequested& GetGlobalStatusRequestedEvent() override
    {
        return OnGlobalStatusRequested;
    }

    virtual FOnEVFileOperationRequested& GetFileOperationRequestedEvent() override
    {
        return OnFileOperationRequested;
    }

    virtual FOnEVNotificationSettingsChangeRequested& GetNotificationSettingsChangeRequestedEvent() override
    {
        return OnNotificationSettingsChangeRequested;
    }

    virtual void ApplyNotificationSettingsState(const FEVPopUpSettingsInfo& Settings) override;

    virtual FOnEVVocabularyValueActionRequested& GetVocabularyValueApplicationRequestedEvent() override
    {
        return OnVocabularyValueActionRequestedByFeature;
    }

    virtual FOnEVApplicationExitRequested& GetApplicationExitRequestedEvent() override
    {
        return OnApplicationExitRequested;
    }

    virtual void ApplyFeatureNavigation(const FEVFeatureNavigationRequest& Request) override;

    // This method is getting called from the PC to
    // confirm a user-selected Pop-up Interval. And it always forces the
    // EEVPopUpIntervals::TurnedOff, regardless of WHAT user has selected other than this option
    // IF the App's Notifications are disabled from within Android or later iOS
    void HandleApplyResolvedPopUpSettings(const FEVPopUpSettingsInfo& PopUpSettingsInfo) override;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    void HandleVocabularySearchRequested(const FEVVocabularySearchRequest& Request);
    void HandleVocabularyRecordRequested(const FEVVocabularyRecordRequest& Request);
    void HandleVocabularyQueryRequested(const FEVVocabularyQueryRequest& Request);
    void HandleVocabularyMutationRequested(const FEVVocabularyMutationRequest& Request);

    void SetupConnectionErrorInfo(FEVErrorInfo& ConnectionErrorInfo);
    bool HandleWidgetControlsState(IEVWidgetControllable* Widget, bool bIsConnectionStatusOnline);
    void HandleOnlineDependantWidgetsActivation(UUserWidget* Widget, bool bIsConnectionStatusOnline);

    UFUNCTION()
    void HandleOnErrorMessageResolved(const FEVErrorInfo& EVErrorInfo);

    UFUNCTION()
    void ButtonMenuPressed();

    UFUNCTION()
    void HandleMenuButtonsPressed(bool bIsAddWordActivated, bool bIsReviewWordsActivated,
                                  bool bIsPopupSettingsActivated, bool bIsImportExportActivated,
                                  bool bIsAppSettingsActivated);

    bool bIsAddWordActivated_internal;
    bool bIsReviewWordsActivated_internal;
    bool bIsPopupSettingsActivated_internal;
    bool bIsImportExportActivated_internal;
    bool bIsAppSettingsActivated_internal;

    UFUNCTION()
    void HandleQuitButtonPressed();

    UFUNCTION()
    void HandleOnAnyWidgetErrorDetected(const FEVErrorInfo& WidgetErrorInfo);

    // We use this handler to invoke the ErrorWidget
    // if any connection issues are detected
    // Interface derrived event
    UPROPERTY(BlueprintAssignable)
    FOnEVError OnRootWidgetError;

    UPROPERTY(BlueprintAssignable)
    FOnLoadingDataTriggerred OnLoadingDataTriggerred;

    UPROPERTY(BlueprintAssignable)
    FOnActionRequested OnActionRequested;

    UPROPERTY(BlueprintAssignable)
    FOnWordEntryWidgetControlsActivated OnWordEntryWidgetControlsActivated;

    UPROPERTY(BlueprintAssignable)
    FOnImportExportDownloadDBOperationIssued OnImportExportDownloadDBOperationIssued;

    UPROPERTY(BlueprintAssignable)
    FOnPopUpIntervalSelectedFromSettings OnPopUpIntervalSelectedFromSettings;

    UPROPERTY(BlueprintAssignable)
    FOnVocabularyValueActionRequested OnVocabularyValueActionRequested;

    UPROPERTY(BlueprintAssignable)
    FOnVocabularyFiltersRequested OnVocabularyFiltersRequested;

    UPROPERTY(BlueprintAssignable)
    FOnVocabularyLanguagePreferencesChangedFromWidgets OnVocabularyLanguagePreferencesChanged;

    FOnEVVocabularySearchRequested OnVocabularySearchRequested;
    FOnEVVocabularyRecordRequested OnVocabularyRecordRequested;
    FOnEVVocabularyQueryRequested OnVocabularyQueryRequested;
    FOnEVVocabularyMutationRequested OnVocabularyMutationRequested;
    FOnEVVocabularyPreferencesChangeRequested OnVocabularyPreferencesChangeRequested;
    FOnEVEntryDetailsRequested OnEntryDetailsRequested;
    FOnEVEntryDetailsCloseRequested OnEntryDetailsCloseRequested;
    FOnEVEntryDetailsDeleteRequested OnEntryDetailsDeleteRequested;
    FOnEVEntryDetailsSaveRequested OnEntryDetailsSaveRequested;
    FOnEVVocabularyFiltersRequested OnVocabularyFiltersRequestedByFeature;
    FOnEVVocabularyFilterCriteriaChangeRequested OnVocabularyFilterCriteriaChangeRequested;
    FOnEVGlobalErrorRequested OnGlobalErrorRequested;
    FOnEVGlobalLoadingStateChanged OnGlobalLoadingStateChanged;
    FOnEVGlobalStatusRequested OnGlobalStatusRequested;
    FOnEVFileOperationRequested OnFileOperationRequested;
    FOnEVNotificationSettingsChangeRequested OnNotificationSettingsChangeRequested;
    FOnEVVocabularyValueActionRequested OnVocabularyValueActionRequestedByFeature;
    FOnEVApplicationExitRequested OnApplicationExitRequested;

    IEVVocabularySearchApplicationPort* AddWordVocabularySearchPort = nullptr;
    IEVVocabularyLibraryApplicationPort* AddWordVocabularyLibraryPort = nullptr;
    IEVVocabularyLibraryApplicationPort* ReviewWordsVocabularyLibraryPort = nullptr;
    IEVFeatureFeedbackSource* AddWordFeedbackSource = nullptr;
    IEVNetworkConnectivityApplicationPort* AddWordConnectivityPort = nullptr;
    IEVWebProviderSelectionConsumer* AddWordWebProviderConsumer = nullptr;
    IEVVocabularyPreferencesFeatureRole* AddWordPreferencesRole = nullptr;
    IEVWordContextFeatureRole* AddWordWordContextRole = nullptr;
    IEVReviewFeatureRole* ReviewFeatureRole = nullptr;
    IEVVocabularyPreferencesFeatureRole* ReviewPreferencesRole = nullptr;
    IEVWordContextFeatureRole* ReviewWordContextRole = nullptr;
    IEVVocabularyPreferencesApplicationPort* SettingsPreferencesPort = nullptr;
    IEVWebProviderSelectionSource* SettingsWebProviderSource = nullptr;
    IEVFileExchangeFeatureRole* FileExchangeFeatureRole = nullptr;
    IEVNotificationSettingsFeatureRole* NotificationSettingsFeatureRole = nullptr;
    IEVMainMenuFeatureRole* MainMenuFeatureRole = nullptr;
    IEVWordEntryDisplayWidgetProvider* EntryDetailsDisplay = nullptr;
    IEVVocabularyFilterWidgetProvider* VocabularyFilterDisplay = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UEVFeatureRegistry> FeatureRegistry;

    FName LastContentFeatureId = EVApplicationFeature::None;
    bool bMainMenuVisible = false;

    void RegisterFeatures();
    bool ActivateFeature(FName FeatureId, bool bRememberCurrent = true);
    void UpdateMainMenuAvailability();
    void HandleFeatureNavigationRequested(FName FeatureId);
    void HandleApplicationExitRequested();
    void HandleFeatureEntryDetailsRequested(const FEVVocabularyRecord& Record);
    void HandleFeatureFiltersRequested();
    void HandleFeatureVocabularyValueActionRequested(const FEVVocabularyValueActionRequest& Request);
    void HandleFeatureFileOperationRequested(const FEVFileOperationInfo& FileOperationInfo);
    void HandleFeatureNotificationSettingsChanged(const FEVPopUpSettingsInfo& Settings);
    void HandleWebProviderSelectionChanged(EEVWebProvider DefinitionProvider, EEVWebProvider TranslationProvider);
    void HandleSettingsPreferencesChangeRequested(const FEVVocabularyPreferencesChangeRequest& Request);

    void HandleEntryDetailsCloseRequested();
    void HandleEntryDetailsEditRequested();
    void HandleEntryDetailsDeleteRequested();
    void HandleEntryDetailsSaveRequested(const FEVVocabularyRecord& Record);

    UFUNCTION()
    void HandleVocabularyFilterCriteriaSubmitted(const FEVVocabularyQueryCriteria& Criteria);

    UFUNCTION()
    void HandleVocabularyFilterDismissed();

    UFUNCTION()
    void HandleOnConnectionErrorDetected();

    // Use this to handle an event fired back from any appropriate widget
    // signaling that the controls have been disabled and further generate the appropriate error widget
    UFUNCTION()
    void HandleOnAnyWidgetControlsDisabled(bool bAreControlsEnabled, const FString& WidgetName);

    UFUNCTION()
    void HandleOnConnectionStateChanged(EEVConnectionState State);

    UFUNCTION()
    void HandleLoadingSpinner(bool bRenderLoadingSpinner);

    UFUNCTION()
    void HandleOnActionRequested(const FEVRequestedActionInfo& RequestedActionInfo);

    UFUNCTION()
    void HandleVocabularyValueActionRequested(const FEVVocabularyValueActionRequest& Request);

    UFUNCTION()
    void HandleVocabularyFiltersRequested();

    UFUNCTION()
    void HandleVocabularyLanguagePreferencesChanged(const FEVVocabularyLanguagePreferences& Preferences);

    UFUNCTION()
    void HandleOnWordEntryWidgetControlsActivated(const FEVWordEntryActionInfo& WordEntryActionInfo);

    UFUNCTION()
    void HandleOnImportExportDownloadDBOperationIssued(const FEVFileOperationInfo& FileOperationInfoFromSelectorWidget);

    UFUNCTION()
    void HandlePopUpIntervalSelected(const FEVPopUpSettingsInfo& PopUpSettingsFromWidget);

    bool bIsAnyMenuActivated;
    int32 MenuSwitcherCount;
    bool bIsAppOnline = false;

    FEVErrorInfo EVConnectionErrorInfo;

    EEVConnectionState EVConnectionState;

    // Connection state color
    static const FName SphereColorParam;
    static const FName OpacityParam;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> ConnectionMID = nullptr;

    void HandleConnectionImageColor(TObjectPtr<UMaterialInstanceDynamic> MaterialInstanceDynamic,
                                    EEVConnectionState ConnectionState);
};
