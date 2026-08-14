// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextWidgetTypes.h"
#include "Components/EditableText.h"
#include "EVSearchResultsPanel.h"
#include "EnhanceVocabularyCore/public/EVVocabularyTypes.h"
#include "EVWidgetControllable.h"
#include "EVWidgetCommonEvents.h"
#include "EVWebProviderTypes.h"
#include "EVVocabularySearchApplicationPort.h"
#include "EVVocabularyLibraryApplicationPort.h"
#include "EVNetworkConnectivityApplicationPort.h"
#include "EVFeatureRoles.h"
#include "EVAddWordWidget.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnError, const FEVErrorInfo&, ErrorInfo);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVAddWordWidget : public UUserWidget,
                                                 public IEVWidgetControllable,
                                                 public IEVWidgetCommonEvents,
                                                 public IEVVocabularySearchApplicationPort,
                                                 public IEVVocabularyLibraryApplicationPort,
                                                 public IEVNetworkConnectivityApplicationPort,
                                                 public IEVFeatureFeedbackSource,
                                                 public IEVWebProviderSelectionConsumer,
                                                 public IEVVocabularyPreferencesFeatureRole,
                                                 public IEVWordContextFeatureRole
{
    GENERATED_BODY()

public:
    // Native UE5.7 widgets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UEditableText* EditableText_WordInput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_Search;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_Clear;

    // The EV app WBPs added to the widget
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UEVSearchResultsPanel> WBP_SearchResultsPanel;

    // Disable/Enable or Get controls status on demand
    virtual void SetControlsEnabled(bool bEnabled) override;
    virtual bool GetControlsEnabled() override;
    void SetInputEnabled(bool bSetInputEnabled);

    /*Events*/

    // Localy constructed event to pass the Storage/Validation/Widget related errors
    UPROPERTY(BlueprintAssignable, Category = "Add Word Widget Events")
    FOnError OnError;

    virtual FOnEVError* GetFeatureErrorEvent() override
    {
        return &OnFeatureError;
    }

    virtual FOnWidgetInteractionDisabled* GetFeatureInteractionDisabledEvent() override
    {
        return &OnWidgetInteractionDisabled;
    }

    virtual FOnLoadingDataTriggerred* GetFeatureLoadingStateEvent() override
    {
        return &OnLoadingDataTriggerred;
    }

    virtual FOnActionRequested* GetFeatureStatusEvent() override
    {
        return &OnActionRequested;
    }

    virtual void ApplyFeatureErrorResolution(const FEVErrorInfo& ErrorInfo) override;
    virtual void ApplyNetworkConnectivityState(EEVApplicationConnectivityState State) override;
    virtual void ApplyWebProviderSelection(EEVWebProvider DefinitionProvider,
                                           EEVWebProvider TranslationProvider) override;
    virtual void ApplyVocabularyPreferences(const FEVVocabularyLanguagePreferences& Preferences) override;
    virtual void PresentWordContext(const FString& Word) override;

    // Common event for the web dependant widgets
    virtual FOnWidgetInteractionDisabled* GetWidgetInteractionDisabledEvent() override
    {
        return &OnWidgetInteractionDisabled;
    }

    virtual FOnLoadingDataTriggerred* GetLoadingSpinnerEvent() override
    {
        return &OnLoadingDataTriggerred;
    }

    virtual FOnActionRequested* GetRequestedActionInfo() override
    {
        return &OnActionRequested;
    }

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
    virtual void ApplyVocabularyQueryOutcome(const FEVVocabularyQueryOutcome&) override {}
    virtual void ApplyVocabularyMutationOutcome(const FEVVocabularyMutationOutcome& Outcome) override;
    virtual void ApplyVocabularyChanged(const FEVVocabularyChangeInfo&) override {}

    // Putting this to public as it's being bound in the EVRootWidget.cpp
    UFUNCTION()
    void HandleWebProvidersChanged(EEVWebProvider DefinitionUsageProvider, EEVWebProvider TranslationProvider);

    UFUNCTION(BlueprintCallable, Category = "Add Word")
    void SetWordInput(const FString& Word);

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    FOnEVError OnFeatureError;

    void Init();

    void ClearStoredSearchResultVariable(FWordSearchResult& CachedWordSearchResult);
    void EnableEditableTextBox(bool bIsEnabled);

    /*Internal elements handlers*/
    UFUNCTION()
    void HandleOnSearchPressed();

    UFUNCTION()
    void HandleOnClearPressed();

    UFUNCTION()
    void HandleEditableTextBoxTextChanged(const FText& NewText);

    FWordSearchResult WordSearchResult;
    FString ThisWidgetName;

    bool bAreInteractionElementsEnabled = true;

    /* Handle events from WBP_SearchResulsPanel*/
    UFUNCTION()
    void HandleOnSaveSearchResultPressed();

    UFUNCTION()
    void HandleSearchWordCompleted(const FWordSearchResult& Result);

    UFUNCTION()
    void HandleOnDiscardSearchResultPressed();

    /* IEVWidgetCommonEvents derrived events*/
    UPROPERTY(BlueprintAssignable)
    FOnWidgetInteractionDisabled OnWidgetInteractionDisabled;

    UPROPERTY(BlueprintAssignable)
    FOnLoadingDataTriggerred OnLoadingDataTriggerred;

    UPROPERTY(BlueprintAssignable)
    FOnActionRequested OnActionRequested;

    FOnEVVocabularySearchRequested OnVocabularySearchRequested;
    FGuid PendingVocabularySearchRequestId;

    FOnEVVocabularyRecordRequested OnVocabularyRecordRequested;
    FOnEVVocabularyQueryRequested OnVocabularyQueryRequested;
    FOnEVVocabularyMutationRequested OnVocabularyMutationRequested;
    FGuid PendingVocabularyRecordRequestId;
    FGuid PendingVocabularyMutationRequestId;

    UFUNCTION()
    void HandleOnWidgetInteractionDisabled();

    UFUNCTION()
    void HandleOnLoadingDataTriggerred(bool ShowLoadingSpinner);

    UFUNCTION()
    void HandleOnActionRequested(const FEVRequestedActionInfo& RequestedActionInfo);

    // Default provider selections used when building vocabulary search requests.
    EEVWebProvider ActiveDefinitionUsageProvider = EEVWebProvider::FreeDictionary;
    EEVWebProvider ActiveTranslationProvider = EEVWebProvider::MyMemory;
};
