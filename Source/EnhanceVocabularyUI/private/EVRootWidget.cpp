// Fill out your copyright notice in the Description page of Project Settings.

#include "EVRootWidget.h"

// Defining consts fpr the Color dynamic material instance params
const FName UEVRootWidget::SphereColorParam(TEXT("SphereColor"));
const FName UEVRootWidget::OpacityParam(TEXT("Opacity"));

void UEVRootWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    SetupConnectionErrorInfo(EVConnectionErrorInfo);

    FeatureRegistry = NewObject<UEVFeatureRegistry>(this);
    if (FeatureRegistry)
    {
        FeatureRegistry->Initialize(WidgetSwitcher_Main);
        RegisterFeatures();
    }

    if (Image_ConnectionState)
    {
        ConnectionMID = Image_ConnectionState->GetDynamicMaterial();
        if (ConnectionMID)
        {
            ConnectionMID->SetScalarParameterValue(OpacityParam, 1.0f);
        }
    }

    bIsAddWordActivated_internal = false;
    bIsReviewWordsActivated_internal = false;
    bIsPopupSettingsActivated_internal = false;
    bIsImportExportActivated_internal = false;
    bIsAppSettingsActivated_internal = false;

    if (AddWord)
    {
        AddWordFeedbackSource = Cast<IEVFeatureFeedbackSource>(AddWord);
        if (AddWordFeedbackSource)
        {
            if (FOnEVError* ErrorEvent = AddWordFeedbackSource->GetFeatureErrorEvent())
            {
                ErrorEvent->AddDynamic(this, &ThisClass::HandleOnAnyWidgetErrorDetected);
            }
            if (FOnWidgetInteractionDisabled* InteractionDisabledEvent =
                    AddWordFeedbackSource->GetFeatureInteractionDisabledEvent())
            {
                InteractionDisabledEvent->AddUniqueDynamic(this, &ThisClass::HandleOnAnyWidgetControlsDisabled);
            }
            if (FOnLoadingDataTriggerred* LoadingStateEvent = AddWordFeedbackSource->GetFeatureLoadingStateEvent())
            {
                LoadingStateEvent->AddUniqueDynamic(this, &ThisClass::HandleLoadingSpinner);
            }
            if (FOnActionRequested* StatusEvent = AddWordFeedbackSource->GetFeatureStatusEvent())
            {
                StatusEvent->AddUniqueDynamic(this, &ThisClass::HandleOnActionRequested);
            }
        }
        else
        {
            AddWord->OnError.AddDynamic(this, &ThisClass::HandleOnAnyWidgetErrorDetected);
        }

        AddWordVocabularySearchPort = Cast<IEVVocabularySearchApplicationPort>(AddWord);
        if (AddWordVocabularySearchPort)
        {
            AddWordVocabularySearchPort->GetVocabularySearchRequestedEvent().AddUObject(
                this, &ThisClass::HandleVocabularySearchRequested);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Add Word does not implement the vocabulary search application port"));
        }

        AddWordVocabularyLibraryPort = Cast<IEVVocabularyLibraryApplicationPort>(AddWord);
        if (AddWordVocabularyLibraryPort)
        {
            AddWordVocabularyLibraryPort->GetVocabularyRecordRequestedEvent().AddUObject(
                this, &ThisClass::HandleVocabularyRecordRequested);
            AddWordVocabularyLibraryPort->GetVocabularyMutationRequestedEvent().AddUObject(
                this, &ThisClass::HandleVocabularyMutationRequested);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Add Word does not implement the vocabulary library application port"));
        }

        if (!AddWordFeedbackSource)
            if (IEVWidgetCommonEvents* WidgetCommonEvents = Cast<IEVWidgetCommonEvents>(AddWord))
            {
                if (FOnWidgetInteractionDisabled* WidgetInputsDisabledEvent =
                        WidgetCommonEvents->GetWidgetInteractionDisabledEvent())
                {
                    WidgetInputsDisabledEvent->AddDynamic(this, &ThisClass::HandleOnAnyWidgetControlsDisabled);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("FOnWidgetInteractionDisabled in EVRootWidget.cpp is nullptr"));
                }

                if (FOnLoadingDataTriggerred* LoadingDataTriggerredEvent = WidgetCommonEvents->GetLoadingSpinnerEvent())
                {
                    LoadingDataTriggerredEvent->AddDynamic(this, &ThisClass::HandleLoadingSpinner);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("FOnLoadingDataTriggerred in EVRootWidget.cpp is nullptr"));
                }

                if (FOnActionRequested* ActionRequested = WidgetCommonEvents->GetRequestedActionInfo())
                {
                    ActionRequested->AddDynamic(this, &ThisClass::HandleOnActionRequested);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("FOnActionRequested in EVRootWidget.cpp is nullptr"));
                }
            }

        AddWordConnectivityPort = Cast<IEVNetworkConnectivityApplicationPort>(AddWord);
        AddWordWebProviderConsumer = Cast<IEVWebProviderSelectionConsumer>(AddWord);
        AddWordPreferencesRole = Cast<IEVVocabularyPreferencesFeatureRole>(AddWord);
        AddWordWordContextRole = Cast<IEVWordContextFeatureRole>(AddWord);
    }

    if (ReviewWords)
    {
        ReviewFeatureRole = Cast<IEVReviewFeatureRole>(ReviewWords);
        if (ReviewFeatureRole)
        {
            ReviewFeatureRole->GetEntryDetailsRequestedEvent().AddUObject(
                this, &ThisClass::HandleFeatureEntryDetailsRequested);
            ReviewFeatureRole->GetFiltersRequestedEvent().AddUObject(this, &ThisClass::HandleFeatureFiltersRequested);
            ReviewFeatureRole->GetVocabularyValueActionRequestedEvent().AddUObject(
                this, &ThisClass::HandleFeatureVocabularyValueActionRequested);
        }
        else
        {
            ReviewWords->OnWordEntryWidgetControlsButtonPressed.AddDynamic(
                this, &ThisClass::HandleOnWordEntryWidgetControlsActivated);
            ReviewWords->OnVocabularyValueActionRequested.AddDynamic(this,
                                                                     &ThisClass::HandleVocabularyValueActionRequested);
            ReviewWords->OnFiltersRequested.AddDynamic(this, &ThisClass::HandleVocabularyFiltersRequested);
        }

        ReviewPreferencesRole = Cast<IEVVocabularyPreferencesFeatureRole>(ReviewWords);
        ReviewWordContextRole = Cast<IEVWordContextFeatureRole>(ReviewWords);

        ReviewWordsVocabularyLibraryPort = Cast<IEVVocabularyLibraryApplicationPort>(ReviewWords);
        if (ReviewWordsVocabularyLibraryPort)
        {
            ReviewWordsVocabularyLibraryPort->GetVocabularyQueryRequestedEvent().AddUObject(
                this, &ThisClass::HandleVocabularyQueryRequested);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Review Words does not implement the vocabulary library application port"));
        }
    }

    if (Settings_SelectWebProviders)
    {
        SettingsPreferencesPort = Cast<IEVVocabularyPreferencesApplicationPort>(Settings_SelectWebProviders);
        SettingsWebProviderSource = Cast<IEVWebProviderSelectionSource>(Settings_SelectWebProviders);

        if (SettingsPreferencesPort)
        {
            SettingsPreferencesPort->GetVocabularyPreferencesChangeRequestedEvent().AddUObject(
                this, &ThisClass::HandleSettingsPreferencesChangeRequested);
        }
        else
        {
            Settings_SelectWebProviders->OnVocabularyLanguagePreferencesChangedSettingsWidget.AddUniqueDynamic(
                this, &ThisClass::HandleVocabularyLanguagePreferencesChanged);
        }

        if (SettingsWebProviderSource)
        {
            SettingsWebProviderSource->GetWebProviderSelectionChangedEvent().AddUObject(
                this, &ThisClass::HandleWebProviderSelectionChanged);
        }
        else if (AddWord)
        {
            Settings_SelectWebProviders->OnWebProvidersSelectionChangedSettingsWidget.AddDynamic(
                AddWord, &UEVAddWordWidget::HandleWebProvidersChanged);
        }
    }

    if (ImportExportDB)
    {
        FileExchangeFeatureRole = Cast<IEVFileExchangeFeatureRole>(ImportExportDB);
        if (FileExchangeFeatureRole)
        {
            FileExchangeFeatureRole->GetFileOperationRequestedEvent().AddUObject(
                this, &ThisClass::HandleFeatureFileOperationRequested);
        }
        else
        {
            ImportExportDB->OnImportExportDownloadDBIssued.AddDynamic(
                this, &ThisClass::HandleOnImportExportDownloadDBOperationIssued);
        }
    }

    if (PopUpSettings)
    {
        NotificationSettingsFeatureRole = Cast<IEVNotificationSettingsFeatureRole>(PopUpSettings);
        if (NotificationSettingsFeatureRole)
        {
            NotificationSettingsFeatureRole->GetNotificationSettingsChangedEvent().AddUObject(
                this, &ThisClass::HandleFeatureNotificationSettingsChanged);
        }
        else
        {
            PopUpSettings->OnNotificationSettingsChanged.AddDynamic(this, &ThisClass::HandlePopUpIntervalSelected);
        }
    }

    if (WordEntryWIdgetDetailedView)
    {
        EntryDetailsDisplay = Cast<IEVWordEntryDisplayWidgetProvider>(WordEntryWIdgetDetailedView);
        if (EntryDetailsDisplay)
        {
            EntryDetailsDisplay->GetViewPressedDelegate().AddUObject(this,
                                                                     &ThisClass::HandleEntryDetailsCloseRequested);
            EntryDetailsDisplay->GetEditPressedDelegate().AddUObject(this, &ThisClass::HandleEntryDetailsEditRequested);
            EntryDetailsDisplay->GetDeletePressedDelegate().AddUObject(this,
                                                                       &ThisClass::HandleEntryDetailsDeleteRequested);
            EntryDetailsDisplay->GetSaveChangesSubmittedDelegate().AddUObject(
                this, &ThisClass::HandleEntryDetailsSaveRequested);
        }
        else
        {
            UE_LOG(LogTemp, Error,
                   TEXT("WordEntryWIdgetDetailedView does not implement the entry-details display role"));
        }
    }

    if (VocabularyFilterWidget)
    {
        VocabularyFilterDisplay = Cast<IEVVocabularyFilterWidgetProvider>(VocabularyFilterWidget);
        if (VocabularyFilterDisplay)
        {
            VocabularyFilterDisplay->GetFiltersAppliedEvent().AddUniqueDynamic(
                this, &ThisClass::HandleVocabularyFilterCriteriaSubmitted);
            VocabularyFilterDisplay->GetCloseRequestedEvent().AddUniqueDynamic(
                this, &ThisClass::HandleVocabularyFilterDismissed);
        }
        else
        {
            UE_LOG(LogTemp, Error,
                   TEXT("VocabularyFilterWidget does not implement the vocabulary-filter display role"));
        }
    }

    if (Button_Menu)
    {
        Button_Menu->OnPressed.AddDynamic(this, &ThisClass::ButtonMenuPressed);
    }

    if (MainMenu)
    {
        MainMenuFeatureRole = Cast<IEVMainMenuFeatureRole>(MainMenu);
        if (MainMenuFeatureRole)
        {
            MainMenuFeatureRole->GetFeatureNavigationRequestedEvent().AddUObject(
                this, &ThisClass::HandleFeatureNavigationRequested);
            MainMenuFeatureRole->GetApplicationExitRequestedEvent().AddUObject(
                this, &ThisClass::HandleApplicationExitRequested);
        }
        else
        {
            MainMenu->OnMenuButtonsPressed.AddDynamic(this, &ThisClass::HandleMenuButtonsPressed);
            MainMenu->OnQuitButtonPressed.AddDynamic(this, &ThisClass::HandleQuitButtonPressed);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to instantiate WBP_MainMenu"));
    }

    UpdateMainMenuAvailability();
}

void UEVRootWidget::RegisterFeatures()
{
    if (!FeatureRegistry)
    {
        return;
    }

    FeatureRegistry->RegisterFeature(EVApplicationFeature::None, NoMenu);
    FeatureRegistry->RegisterFeature(EVApplicationFeature::MainMenu, MainMenu);
    FeatureRegistry->RegisterFeature(EVApplicationFeature::AddWord, AddWord);
    FeatureRegistry->RegisterFeature(EVApplicationFeature::ReviewWords, ReviewWords);
    FeatureRegistry->RegisterFeature(EVApplicationFeature::NotificationSettings, PopUpSettings);
    FeatureRegistry->RegisterFeature(EVApplicationFeature::ImportExport, ImportExportDB);
    FeatureRegistry->RegisterFeature(EVApplicationFeature::ApplicationSettings, Settings_SelectWebProviders);
    FeatureRegistry->RegisterFeature(EVApplicationFeature::EntryDetails, WordEntryWIdgetDetailedView);
    FeatureRegistry->RegisterFeature(EVApplicationFeature::VocabularyFilters, VocabularyFilterWidget);
}

bool UEVRootWidget::ActivateFeature(const FName FeatureId, const bool bRememberCurrent)
{
    if (!FeatureRegistry || !FeatureRegistry->ActivateFeature(FeatureId, bRememberCurrent))
    {
        UE_LOG(LogTemp, Warning, TEXT("Requested Feature is unavailable: %s"), *FeatureId.ToString());
        return false;
    }

    if (FeatureId != EVApplicationFeature::MainMenu && FeatureId != EVApplicationFeature::EntryDetails &&
        FeatureId != EVApplicationFeature::VocabularyFilters)
    {
        LastContentFeatureId = FeatureId;
    }

    return true;
}

void UEVRootWidget::UpdateMainMenuAvailability()
{
    MainMenuFeatureRole = Cast<IEVMainMenuFeatureRole>(MainMenu);
    if (!MainMenuFeatureRole || !FeatureRegistry)
    {
        return;
    }

    MainMenuFeatureRole->SetFeatureAvailable(EVApplicationFeature::AddWord,
                                             FeatureRegistry->IsFeatureAvailable(EVApplicationFeature::AddWord));
    MainMenuFeatureRole->SetFeatureAvailable(EVApplicationFeature::ReviewWords,
                                             FeatureRegistry->IsFeatureAvailable(EVApplicationFeature::ReviewWords));
    MainMenuFeatureRole->SetFeatureAvailable(
        EVApplicationFeature::NotificationSettings,
        FeatureRegistry->IsFeatureAvailable(EVApplicationFeature::NotificationSettings));
    MainMenuFeatureRole->SetFeatureAvailable(EVApplicationFeature::ImportExport,
                                             FeatureRegistry->IsFeatureAvailable(EVApplicationFeature::ImportExport));
    MainMenuFeatureRole->SetFeatureAvailable(
        EVApplicationFeature::ApplicationSettings,
        FeatureRegistry->IsFeatureAvailable(EVApplicationFeature::ApplicationSettings));
}

void UEVRootWidget::HandleFeatureNavigationRequested(const FName FeatureId)
{
    if (FeatureId == EVApplicationFeature::AddWord && !bIsAppOnline)
    {
        HandleOnConnectionErrorDetected();
        return;
    }

    if (!ActivateFeature(FeatureId))
    {
        return;
    }

    bMainMenuVisible = false;
    MenuSwitcherCount = 0;
    bIsAnyMenuActivated = FeatureId != EVApplicationFeature::None;
    bIsAddWordActivated_internal = FeatureId == EVApplicationFeature::AddWord;
    bIsReviewWordsActivated_internal = FeatureId == EVApplicationFeature::ReviewWords;
    bIsPopupSettingsActivated_internal = FeatureId == EVApplicationFeature::NotificationSettings;
    bIsImportExportActivated_internal = FeatureId == EVApplicationFeature::ImportExport;
    bIsAppSettingsActivated_internal = FeatureId == EVApplicationFeature::ApplicationSettings;

    if (FeatureId == EVApplicationFeature::ReviewWords && ReviewFeatureRole)
    {
        ReviewFeatureRole->RefreshFeature();
    }
}

void UEVRootWidget::HandleApplicationExitRequested()
{
    OnApplicationExitRequested.Broadcast();
}

void UEVRootWidget::HandleFeatureEntryDetailsRequested(const FEVVocabularyRecord& Record)
{
    OnEntryDetailsRequested.Broadcast(Record);
}

void UEVRootWidget::HandleFeatureFiltersRequested()
{
    if (OnVocabularyFiltersRequestedByFeature.IsBound())
    {
        OnVocabularyFiltersRequestedByFeature.Broadcast();
    }
    else
    {
        OnVocabularyFiltersRequested.Broadcast();
    }
}

void UEVRootWidget::HandleFeatureVocabularyValueActionRequested(const FEVVocabularyValueActionRequest& Request)
{
    if (OnVocabularyValueActionRequestedByFeature.IsBound())
    {
        OnVocabularyValueActionRequestedByFeature.Broadcast(Request);
    }
    else
    {
        OnVocabularyValueActionRequested.Broadcast(Request);
    }
}

void UEVRootWidget::HandleFeatureFileOperationRequested(const FEVFileOperationInfo& FileOperationInfo)
{
    if (OnFileOperationRequested.IsBound())
    {
        OnFileOperationRequested.Broadcast(FileOperationInfo);
    }
    else
    {
        OnImportExportDownloadDBOperationIssued.Broadcast(FileOperationInfo);
    }
}

void UEVRootWidget::HandleFeatureNotificationSettingsChanged(const FEVPopUpSettingsInfo& Settings)
{
    if (OnNotificationSettingsChangeRequested.IsBound())
    {
        OnNotificationSettingsChangeRequested.Broadcast(Settings);
    }
    else
    {
        OnPopUpIntervalSelectedFromSettings.Broadcast(Settings);
    }
}

void UEVRootWidget::HandleWebProviderSelectionChanged(const EEVWebProvider DefinitionProvider,
                                                      const EEVWebProvider TranslationProvider)
{
    if (AddWordWebProviderConsumer)
    {
        AddWordWebProviderConsumer->ApplyWebProviderSelection(DefinitionProvider, TranslationProvider);
    }
}

void UEVRootWidget::HandleSettingsPreferencesChangeRequested(const FEVVocabularyPreferencesChangeRequest& Request)
{
    OnVocabularyPreferencesChangeRequested.Broadcast(Request);
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
        if (!ActivateFeature(EVApplicationFeature::None, false))
        {
            WidgetSwitcher_Main->SetActiveWidget(NoMenu);
        }
        bIsAnyMenuActivated = false;
        MenuSwitcherCount = 0;
        bMainMenuVisible = false;
    }
}

void UEVRootWidget::NativeDestruct()
{
    if (AddWord)
    {
        AddWord->OnError.RemoveAll(this);
    }

    if (Settings_SelectWebProviders)
    {
        Settings_SelectWebProviders->OnVocabularyLanguagePreferencesChangedSettingsWidget.RemoveAll(this);
        if (AddWord)
        {
            Settings_SelectWebProviders->OnWebProvidersSelectionChangedSettingsWidget.RemoveAll(AddWord);
        }
    }

    if (ImportExportDB)
    {
        ImportExportDB->OnImportExportDownloadDBIssued.RemoveAll(this);
    }

    if (PopUpSettings)
    {
        PopUpSettings->OnNotificationSettingsChanged.RemoveAll(this);
    }

    if (MainMenu)
    {
        MainMenu->OnMenuButtonsPressed.RemoveAll(this);
        MainMenu->OnQuitButtonPressed.RemoveAll(this);
    }

    if (AddWordVocabularySearchPort)
    {
        AddWordVocabularySearchPort->GetVocabularySearchRequestedEvent().RemoveAll(this);
        AddWordVocabularySearchPort = nullptr;
    }

    if (AddWordVocabularyLibraryPort)
    {
        AddWordVocabularyLibraryPort->GetVocabularyRecordRequestedEvent().RemoveAll(this);
        AddWordVocabularyLibraryPort->GetVocabularyMutationRequestedEvent().RemoveAll(this);
        AddWordVocabularyLibraryPort = nullptr;
    }

    if (ReviewWordsVocabularyLibraryPort)
    {
        ReviewWordsVocabularyLibraryPort->GetVocabularyQueryRequestedEvent().RemoveAll(this);
        ReviewWordsVocabularyLibraryPort = nullptr;
    }

    if (ReviewFeatureRole)
    {
        ReviewFeatureRole->GetEntryDetailsRequestedEvent().RemoveAll(this);
        ReviewFeatureRole->GetFiltersRequestedEvent().RemoveAll(this);
        ReviewFeatureRole->GetVocabularyValueActionRequestedEvent().RemoveAll(this);
    }

    if (SettingsPreferencesPort)
    {
        SettingsPreferencesPort->GetVocabularyPreferencesChangeRequestedEvent().RemoveAll(this);
    }

    if (SettingsWebProviderSource)
    {
        SettingsWebProviderSource->GetWebProviderSelectionChangedEvent().RemoveAll(this);
    }

    if (FileExchangeFeatureRole)
    {
        FileExchangeFeatureRole->GetFileOperationRequestedEvent().RemoveAll(this);
    }

    if (NotificationSettingsFeatureRole)
    {
        NotificationSettingsFeatureRole->GetNotificationSettingsChangedEvent().RemoveAll(this);
    }

    if (MainMenuFeatureRole)
    {
        MainMenuFeatureRole->GetFeatureNavigationRequestedEvent().RemoveAll(this);
        MainMenuFeatureRole->GetApplicationExitRequestedEvent().RemoveAll(this);
    }

    if (EntryDetailsDisplay)
    {
        EntryDetailsDisplay->GetViewPressedDelegate().RemoveAll(this);
        EntryDetailsDisplay->GetEditPressedDelegate().RemoveAll(this);
        EntryDetailsDisplay->GetDeletePressedDelegate().RemoveAll(this);
        EntryDetailsDisplay->GetSaveChangesSubmittedDelegate().RemoveAll(this);
    }

    if (VocabularyFilterDisplay)
    {
        VocabularyFilterDisplay->GetFiltersAppliedEvent().RemoveAll(this);
        VocabularyFilterDisplay->GetCloseRequestedEvent().RemoveAll(this);
    }

    AddWordFeedbackSource = nullptr;
    AddWordConnectivityPort = nullptr;
    AddWordWebProviderConsumer = nullptr;
    AddWordPreferencesRole = nullptr;
    AddWordWordContextRole = nullptr;
    ReviewFeatureRole = nullptr;
    ReviewPreferencesRole = nullptr;
    ReviewWordContextRole = nullptr;
    SettingsPreferencesPort = nullptr;
    SettingsWebProviderSource = nullptr;
    FileExchangeFeatureRole = nullptr;
    NotificationSettingsFeatureRole = nullptr;
    MainMenuFeatureRole = nullptr;
    EntryDetailsDisplay = nullptr;
    VocabularyFilterDisplay = nullptr;

    Super::NativeDestruct();
}

void UEVRootWidget::HandleVocabularySearchRequested(const FEVVocabularySearchRequest& Request)
{
    OnVocabularySearchRequested.Broadcast(Request);
}

void UEVRootWidget::HandleVocabularyRecordRequested(const FEVVocabularyRecordRequest& Request)
{
    OnVocabularyRecordRequested.Broadcast(Request);
}

void UEVRootWidget::HandleVocabularyQueryRequested(const FEVVocabularyQueryRequest& Request)
{
    OnVocabularyQueryRequested.Broadcast(Request);
}

void UEVRootWidget::HandleVocabularyMutationRequested(const FEVVocabularyMutationRequest& Request)
{
    OnVocabularyMutationRequested.Broadcast(Request);
}

void UEVRootWidget::PresentEntryDetails(const FEVVocabularyRecord& Record)
{
    if (!WidgetSwitcher_Main || !WordEntryWIdgetDetailedView || !EntryDetailsDisplay)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot present Entry Details: required Root Feature binding is missing."));
        return;
    }

    EntryDetailsDisplay->ShowWordEntry(Record);
    EntryDetailsDisplay->SetEditableFieldsReadOnly(true);
    EntryDetailsDisplay->SetButtonsDisabled(false, false, false, true);
    if (!ActivateFeature(EVApplicationFeature::EntryDetails))
    {
        WidgetSwitcher_Main->SetActiveWidget(WordEntryWIdgetDetailedView);
    }
}

void UEVRootWidget::DismissEntryDetails()
{
    if (FeatureRegistry && FeatureRegistry->ReturnToPreviousFeature())
    {
        return;
    }

    if (!ActivateFeature(EVApplicationFeature::ReviewWords, false) && WidgetSwitcher_Main && ReviewWords)
    {
        WidgetSwitcher_Main->SetActiveWidget(ReviewWords);
    }
}

void UEVRootWidget::PresentVocabularyFilters(const FEVVocabularyQueryCriteria& Criteria)
{
    if (!WidgetSwitcher_Main || !VocabularyFilterWidget || !VocabularyFilterDisplay)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot present Vocabulary Filters: required Root Feature binding is missing."));
        return;
    }

    VocabularyFilterDisplay->SetInitialCriteria(Criteria);
    if (!ActivateFeature(EVApplicationFeature::VocabularyFilters))
    {
        WidgetSwitcher_Main->SetActiveWidget(VocabularyFilterWidget);
    }
}

void UEVRootWidget::ApplyVocabularyFilterCriteria(const FEVVocabularyQueryCriteria& Criteria)
{
    HandleVocabularyFiltersApplied(Criteria);

    if (FeatureRegistry && FeatureRegistry->ReturnToPreviousFeature())
    {
        return;
    }

    if (!ActivateFeature(EVApplicationFeature::ReviewWords, false) && WidgetSwitcher_Main && ReviewWords)
    {
        WidgetSwitcher_Main->SetActiveWidget(ReviewWords);
    }
}

void UEVRootWidget::ApplyVocabularySearchOutcome(const FEVVocabularySearchOutcome& Outcome)
{
    if (AddWordVocabularySearchPort)
    {
        AddWordVocabularySearchPort->ApplyVocabularySearchOutcome(Outcome);
    }
}

void UEVRootWidget::ApplyVocabularyRecordOutcome(const FEVVocabularyRecordOutcome& Outcome)
{
    if (AddWordVocabularyLibraryPort)
    {
        AddWordVocabularyLibraryPort->ApplyVocabularyRecordOutcome(Outcome);
    }
}

void UEVRootWidget::ApplyVocabularyQueryOutcome(const FEVVocabularyQueryOutcome& Outcome)
{
    if (ReviewWordsVocabularyLibraryPort)
    {
        ReviewWordsVocabularyLibraryPort->ApplyVocabularyQueryOutcome(Outcome);
    }
}

void UEVRootWidget::ApplyVocabularyMutationOutcome(const FEVVocabularyMutationOutcome& Outcome)
{
    if (AddWordVocabularyLibraryPort)
    {
        AddWordVocabularyLibraryPort->ApplyVocabularyMutationOutcome(Outcome);
    }
}

void UEVRootWidget::ApplyVocabularyChanged(const FEVVocabularyChangeInfo& ChangeInfo)
{
    if (ReviewFeatureRole)
    {
        ReviewFeatureRole->ApplyVocabularyChange(ChangeInfo);
    }
}

void UEVRootWidget::ApplyVocabularyPreferencesState(const FEVVocabularyPreferencesState& State)
{
    if (SettingsPreferencesPort)
    {
        SettingsPreferencesPort->ApplyVocabularyPreferencesState(State);
    }

    if (State.Result != EEVApplicationOperationResult::Succeeded)
    {
        return;
    }

    if (AddWordPreferencesRole)
    {
        AddWordPreferencesRole->ApplyVocabularyPreferences(State.Preferences);
    }
    if (ReviewPreferencesRole)
    {
        ReviewPreferencesRole->ApplyVocabularyPreferences(State.Preferences);
    }
}

void UEVRootWidget::ApplyNetworkConnectivityState(const EEVApplicationConnectivityState State)
{
    if (AddWordConnectivityPort)
    {
        AddWordConnectivityPort->ApplyNetworkConnectivityState(State);
    }

    switch (State)
    {
    case EEVApplicationConnectivityState::Online:
        HandleOnConnectionStateChanged(EEVConnectionState::Online);
        break;

    case EEVApplicationConnectivityState::Connecting:
        HandleOnConnectionStateChanged(EEVConnectionState::Connecting);
        break;

    default:
        HandleOnConnectionStateChanged(EEVConnectionState::Offline);
        break;
    }
}

void UEVRootWidget::ApplyGlobalErrorResolution(const FEVErrorInfo& ErrorInfo)
{
    if (AddWordFeedbackSource)
    {
        AddWordFeedbackSource->ApplyFeatureErrorResolution(ErrorInfo);
    }
    HandleOnErrorMessageResolved(ErrorInfo);
}

void UEVRootWidget::ApplyNotificationSettingsState(const FEVPopUpSettingsInfo& Settings)
{
    if (NotificationSettingsFeatureRole)
    {
        NotificationSettingsFeatureRole->ApplyNotificationSettings(Settings);
        return;
    }

    HandleApplyResolvedPopUpSettings(Settings);
}

void UEVRootWidget::ApplyFeatureNavigation(const FEVFeatureNavigationRequest& Request)
{
    if (!ActivateFeature(Request.FeatureId))
    {
        return;
    }

    if (!Request.Context.IsEmpty())
    {
        if (Request.FeatureId == EVApplicationFeature::ReviewWords && ReviewWordContextRole)
        {
            ReviewWordContextRole->PresentWordContext(Request.Context);
        }
        else if (Request.FeatureId == EVApplicationFeature::AddWord && AddWordWordContextRole)
        {
            AddWordWordContextRole->PresentWordContext(Request.Context);
        }
    }
}

void UEVRootWidget::ButtonMenuPressed()
{
    if (!bMainMenuVisible)
    {
        if (!ActivateFeature(EVApplicationFeature::MainMenu, false))
        {
            return;
        }

        bMainMenuVisible = true;
        MenuSwitcherCount = 1;
        return;
    }

    bMainMenuVisible = false;
    MenuSwitcherCount = 0;

    const FName Destination = LastContentFeatureId.IsNone() ? EVApplicationFeature::None : LastContentFeatureId;
    if (!ActivateFeature(Destination, false))
    {
        ActivateFeature(EVApplicationFeature::None, false);
    }
}

void UEVRootWidget::HandleMenuButtonsPressed(bool bIsAddWordActivated, bool bIsReviewWordsActivated,
                                             bool bIsPopupSettingsActivated, bool bIsImportExportActivated,
                                             bool bIsAppSettingsActivated)
{
    FName RequestedFeature = EVApplicationFeature::None;
    if (bIsAddWordActivated)
    {
        RequestedFeature = EVApplicationFeature::AddWord;
    }
    else if (bIsReviewWordsActivated)
    {
        RequestedFeature = EVApplicationFeature::ReviewWords;
    }
    else if (bIsPopupSettingsActivated)
    {
        RequestedFeature = EVApplicationFeature::NotificationSettings;
    }
    else if (bIsAppSettingsActivated)
    {
        RequestedFeature = EVApplicationFeature::ApplicationSettings;
    }
    else if (bIsImportExportActivated)
    {
        RequestedFeature = EVApplicationFeature::ImportExport;
    }

    if (RequestedFeature != EVApplicationFeature::None)
    {
        HandleFeatureNavigationRequested(RequestedFeature);
    }
}

void UEVRootWidget::SetupConnectionErrorInfo(FEVErrorInfo& ConnectionErrorInfo)
{
    ConnectionErrorInfo.Source = EEVErrorSource::ConnectionModule;
    ConnectionErrorInfo.Type = EEVErrorType::ConnectionError;
    ConnectionErrorInfo.Message = FText::FromString(
        TEXT("Failed to connect to the WEB. You are Offline and some functionality may not be available"));
}

bool UEVRootWidget::HandleWidgetControlsState(IEVWidgetControllable* Widget, bool bIsConnectionStatusOnline)
{
    if (Widget)
    {
        if (bIsConnectionStatusOnline == true && Widget->GetControlsEnabled() == true)
        {
            return true;
        }
        else if (bIsConnectionStatusOnline == false && Widget->GetControlsEnabled() == true)
        {
            Widget->SetControlsEnabled(false);
            return false;
        }
        else if (bIsConnectionStatusOnline == true && Widget->GetControlsEnabled() == false)
        {
            Widget->SetControlsEnabled(true);
            return true;
        }

        return false;
    }

    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get instance of IEVWidgetControllable"));
        return false;
    }
}

void UEVRootWidget::HandleOnlineDependantWidgetsActivation(UUserWidget* Widget, bool bIsConnectionStatusOnline)
{
    if (Widget)
    {
        if (bIsConnectionStatusOnline)
        {
            WidgetSwitcher_Main->SetActiveWidget(Widget);
        }
        else
        {
            HandleOnConnectionErrorDetected();
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get instance of UUserWidget"));
    }
}

void UEVRootWidget::HandleOnErrorMessageResolved(const FEVErrorInfo& EVErrorInfo)
{
    EEVErrorSource EVErrorSource = EVErrorInfo.Source;
    EEVErrorType EEVErrorType = EVErrorInfo.Type;

    switch (EVErrorSource)
    {
    case EEVErrorSource::AddWord:
        if (EEVErrorType == EEVErrorType::SearchError)
        {
            if (AddWordFeedbackSource)
            {
                AddWordFeedbackSource->ApplyFeatureErrorResolution(EVErrorInfo);
            }
            else if (AddWord)
            {
                AddWord->SetInputEnabled(true);
            }
        }
        break;
    default:
        break;
    }
}

void UEVRootWidget::HandleQuitButtonPressed()
{
    HandleApplicationExitRequested();
}

void UEVRootWidget::HandleOnAnyWidgetErrorDetected(const FEVErrorInfo& WidgetErrorInfo)
{
    if (OnGlobalErrorRequested.IsBound())
    {
        OnGlobalErrorRequested.Broadcast(WidgetErrorInfo);
    }
    else
    {
        OnRootWidgetError.Broadcast(WidgetErrorInfo);
    }
}

void UEVRootWidget::HandleOnConnectionErrorDetected()
{
    if (OnGlobalErrorRequested.IsBound())
    {
        OnGlobalErrorRequested.Broadcast(EVConnectionErrorInfo);
    }
    else
    {
        OnRootWidgetError.Broadcast(EVConnectionErrorInfo);
    }
}

void UEVRootWidget::HandleOnAnyWidgetControlsDisabled(bool bAreControlsEnabled, const FString& WidgetName)
{
    if (!bAreControlsEnabled)
    {
        if (EVConnectionState != EEVConnectionState::Online)
        {
            HandleOnConnectionErrorDetected();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("The %s widget controls disabled but we are Online"), *WidgetName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("The %s widget controls were not disabled"), *WidgetName);
    }
}

void UEVRootWidget::HandleOnConnectionStateChanged(EEVConnectionState State)
{
    EVConnectionState = State;

    switch (State)
    {
    case EEVConnectionState::Offline:
        UE_LOG(LogTemp, Warning, TEXT("We are Offline"));
        bIsAppOnline = false;
        if (!AddWordConnectivityPort)
        {
            HandleWidgetControlsState(AddWord, bIsAppOnline);
        }
        HandleOnConnectionErrorDetected();
        HandleConnectionImageColor(ConnectionMID, State);
        break;
    case EEVConnectionState::Connecting:
        bIsAppOnline = false;
        if (!AddWordConnectivityPort)
        {
            HandleWidgetControlsState(AddWord, bIsAppOnline);
        }
        HandleConnectionImageColor(ConnectionMID, State);
        UE_LOG(LogTemp, Warning, TEXT("We are Connecting"));
        break;
    case EEVConnectionState::Online:
        bIsAppOnline = true;
        if (!AddWordConnectivityPort)
        {
            HandleWidgetControlsState(AddWord, bIsAppOnline);
        }
        HandleConnectionImageColor(ConnectionMID, State);
        UE_LOG(LogTemp, Warning, TEXT("We are Online"));
        break;
    default:
        HandleConnectionImageColor(ConnectionMID, State);
        break;
    }
}

void UEVRootWidget::HandleLoadingSpinner(bool bRenderLoadingSpinner)
{
    if (OnGlobalLoadingStateChanged.IsBound())
    {
        OnGlobalLoadingStateChanged.Broadcast(bRenderLoadingSpinner);
    }
    else
    {
        OnLoadingDataTriggerred.Broadcast(bRenderLoadingSpinner);
    }
}

void UEVRootWidget::HandleOnActionRequested(const FEVRequestedActionInfo& RequestedActionInfo)
{
    if (OnGlobalStatusRequested.IsBound())
    {
        OnGlobalStatusRequested.Broadcast(RequestedActionInfo);
    }
    else
    {
        OnActionRequested.Broadcast(RequestedActionInfo);
    }
}

void UEVRootWidget::HandleEntryDetailsCloseRequested()
{
    OnEntryDetailsCloseRequested.Broadcast();
}

void UEVRootWidget::HandleEntryDetailsEditRequested()
{
    if (EntryDetailsDisplay)
    {
        EntryDetailsDisplay->SetButtonsDisabled(true, true, true, false);
        EntryDetailsDisplay->SetEditableFieldsReadOnly(false);
    }
}

void UEVRootWidget::HandleEntryDetailsDeleteRequested()
{
    OnEntryDetailsDeleteRequested.Broadcast();
}

void UEVRootWidget::HandleEntryDetailsSaveRequested(const FEVVocabularyRecord& Record)
{
    OnEntryDetailsSaveRequested.Broadcast(Record);
}

void UEVRootWidget::HandleVocabularyFilterCriteriaSubmitted(const FEVVocabularyQueryCriteria& Criteria)
{
    OnVocabularyFilterCriteriaChangeRequested.Broadcast(Criteria);
}

void UEVRootWidget::HandleVocabularyFilterDismissed()
{
    if (FeatureRegistry && FeatureRegistry->ReturnToPreviousFeature())
    {
        return;
    }

    if (!ActivateFeature(EVApplicationFeature::ReviewWords, false) && WidgetSwitcher_Main && ReviewWords)
    {
        WidgetSwitcher_Main->SetActiveWidget(ReviewWords);
    }
}

void UEVRootWidget::HandleOnWordEntryWidgetControlsActivated(const FEVWordEntryActionInfo& WordEntryActionInfo)
{
    OnWordEntryWidgetControlsActivated.Broadcast(WordEntryActionInfo);
}

void UEVRootWidget::HandleOnImportExportDownloadDBOperationIssued(
    const FEVFileOperationInfo& FileOperationInfoFromSelectorWidget)
{
    if (OnFileOperationRequested.IsBound())
    {
        OnFileOperationRequested.Broadcast(FileOperationInfoFromSelectorWidget);
    }
    else
    {
        OnImportExportDownloadDBOperationIssued.Broadcast(FileOperationInfoFromSelectorWidget);
    }
}

void UEVRootWidget::HandlePopUpIntervalSelected(const FEVPopUpSettingsInfo& PopUpSettingsFromWidget)
{
    if (OnNotificationSettingsChangeRequested.IsBound())
    {
        OnNotificationSettingsChangeRequested.Broadcast(PopUpSettingsFromWidget);
    }
    else
    {
        OnPopUpIntervalSelectedFromSettings.Broadcast(PopUpSettingsFromWidget);
    }
}

void UEVRootWidget::HandleReviewWordsRefresh()
{
    if (ReviewFeatureRole)
    {
        ReviewFeatureRole->RefreshFeature();
        return;
    }

    if (!ReviewWords)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot refresh Review Words: ReviewWords is null."));

        return;
    }

    ReviewWords->RefreshReview();
}

void UEVRootWidget::HandleOpenReviewWordsForNotification(const FString& Word)
{
    if (ReviewWordContextRole)
    {
        FEVFeatureNavigationRequest Request;
        Request.FeatureId = EVApplicationFeature::ReviewWords;
        Request.Context = Word;
        ApplyFeatureNavigation(Request);
        return;
    }

    if (!WidgetSwitcher_Main || !ReviewWords)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot open Review Words from notification: required widget is null."));
        return;
    }

    bIsAnyMenuActivated = true;
    bIsAddWordActivated_internal = false;
    bIsReviewWordsActivated_internal = true;
    bIsPopupSettingsActivated_internal = false;
    bIsImportExportActivated_internal = false;
    bIsAppSettingsActivated_internal = false;
    MenuSwitcherCount = 0;

    WidgetSwitcher_Main->SetActiveWidget(ReviewWords);
    ReviewWords->SetSearchWord(Word);
}

void UEVRootWidget::HandleVocabularyValueActionRequested(const FEVVocabularyValueActionRequest& Request)
{
    if (OnVocabularyValueActionRequestedByFeature.IsBound())
    {
        OnVocabularyValueActionRequestedByFeature.Broadcast(Request);
    }
    else
    {
        OnVocabularyValueActionRequested.Broadcast(Request);
    }
}

void UEVRootWidget::HandleOpenAddWordWithWord(const FString& Word)
{
    if (AddWordWordContextRole)
    {
        FEVFeatureNavigationRequest Request;
        Request.FeatureId = EVApplicationFeature::AddWord;
        Request.Context = Word;
        ApplyFeatureNavigation(Request);
        return;
    }

    if (!WidgetSwitcher_Main || !AddWord)
    {
        return;
    }

    bIsAnyMenuActivated = true;
    bIsAddWordActivated_internal = true;
    bIsReviewWordsActivated_internal = false;
    bIsPopupSettingsActivated_internal = false;
    bIsImportExportActivated_internal = false;
    bIsAppSettingsActivated_internal = false;
    MenuSwitcherCount = 0;

    HandleOnlineDependantWidgetsActivation(AddWord, bIsAppOnline);
    HandleWidgetControlsState(AddWord, bIsAppOnline);
    AddWord->SetWordInput(Word);
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

void UEVRootWidget::HandleApplyResolvedPopUpSettings(const FEVPopUpSettingsInfo& PopUpSettingsInfo)
{
    if (NotificationSettingsFeatureRole)
    {
        NotificationSettingsFeatureRole->ApplyNotificationSettings(PopUpSettingsInfo);
        return;
    }

    if (!PopUpSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot apply resolved pop-up settings: PopUpSettings is null."));

        return;
    }

    PopUpSettings->SetSelectedSettings(PopUpSettingsInfo);
}

void UEVRootWidget::HandleWordEntryChanged(const FEVWordEntryActionInfo& WordEntryActionInfo)
{
    if (ReviewFeatureRole)
    {
        FEVVocabularyChangeInfo ChangeInfo;
        ChangeInfo.ChangeId = FGuid::NewGuid();
        ChangeInfo.ChangeType = WordEntryActionInfo.ActionType == EEVWordEntryActionType::DeleteEntry
                                    ? EEVVocabularyChangeType::Removed
                                    : EEVVocabularyChangeType::Updated;
        ChangeInfo.AffectedNormalizedWords.Add(WordEntryActionInfo.EntryInfo.NormalizedWord.IsEmpty()
                                                   ? WordEntryActionInfo.EntryInfo.Word
                                                   : WordEntryActionInfo.EntryInfo.NormalizedWord);
        ReviewFeatureRole->ApplyVocabularyChange(ChangeInfo);
        return;
    }

    if (!ReviewWords)
    {
        return;
    }

    switch (WordEntryActionInfo.ActionType)
    {
    case EEVWordEntryActionType::SaveEditedEntry:
        ReviewWords->UpdateDisplayedWordEntry(WordEntryActionInfo.EntryInfo);
        break;

    case EEVWordEntryActionType::DeleteEntry:
        ReviewWords->RemoveDisplayedWordEntry(WordEntryActionInfo.EntryInfo);
        break;

    default:
        break;
    }
}

void UEVRootWidget::HandleVocabularyFiltersRequested()
{
    if (OnVocabularyFiltersRequestedByFeature.IsBound())
    {
        OnVocabularyFiltersRequestedByFeature.Broadcast();
    }
    else
    {
        OnVocabularyFiltersRequested.Broadcast();
    }
}

void UEVRootWidget::HandleVocabularyFiltersApplied(const FEVVocabularyQueryCriteria& Criteria)
{
    if (ReviewFeatureRole)
    {
        ReviewFeatureRole->ApplyQueryCriteria(Criteria);
    }
    else if (ReviewWords)
    {
        ReviewWords->ApplyQueryCriteria(Criteria);
    }
}

void UEVRootWidget::HandleVocabularyLanguagePreferencesChanged(const FEVVocabularyLanguagePreferences& Preferences)
{
    FEVVocabularyPreferencesChangeRequest Request;
    Request.RequestId = FGuid::NewGuid();
    Request.Preferences = Preferences;
    if (OnVocabularyPreferencesChangeRequested.IsBound())
    {
        OnVocabularyPreferencesChangeRequested.Broadcast(Request);
    }
    else
    {
        OnVocabularyLanguagePreferencesChanged.Broadcast(Preferences);
    }
}

void UEVRootWidget::HandleVocabularyLanguagePreferencesApplied(const FEVVocabularyLanguagePreferences& Preferences)
{
    FEVVocabularyPreferencesState State;
    State.Preferences = Preferences;

    if (SettingsPreferencesPort)
    {
        SettingsPreferencesPort->ApplyVocabularyPreferencesState(State);
    }
    else if (Settings_SelectWebProviders)
    {
        Settings_SelectWebProviders->ApplyVocabularyLanguagePreferences(Preferences);
    }

    if (AddWordPreferencesRole)
    {
        AddWordPreferencesRole->ApplyVocabularyPreferences(Preferences);
    }

    if (ReviewPreferencesRole)
    {
        ReviewPreferencesRole->ApplyVocabularyPreferences(Preferences);
    }
    else if (ReviewWords)
    {
        ReviewWords->RefreshReview();
    }
}
