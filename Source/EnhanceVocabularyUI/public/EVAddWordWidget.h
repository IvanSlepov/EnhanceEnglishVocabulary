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
#include "EVAddWordWidget.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnError, const FEVErrorInfo&, ErrorInfo);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVAddWordWidget : public UUserWidget,
                                                 public IEVWidgetControllable,
                                                 public IEVWidgetCommonEvents
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UEVSearchResultsPanel> WBP_SearchResultsPanel;

    class UEVGameInstance* EVGameInstance;

    // Disable/Enable or Get controls status on demand
    virtual void SetControlsEnabled(bool bEnabled) override;
    virtual bool GetControlsEnabled() override;

    /*Events*/

    // Localy constructed event to pass the Storage/Validation/Widget related errors
    UPROPERTY(BlueprintAssignable, Category = "Add Word Widget Events")
    FOnError OnError;

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

    // Putting this to public as it's being bound in the EVRootWidget.cpp
    UFUNCTION()
    void HandleWebProvidersChanged(EEVWebProvider DefinitionUsageProvider, EEVWebProvider TranslationProvider);

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
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

    UFUNCTION()
    void HandleOnWidgetInteractionDisabled();

    UFUNCTION()
    void HandleOnLoadingDataTriggerred(bool ShowLoadingSpinner);

    UFUNCTION()
    void HandleOnActionRequested(const FEVRequestedActionInfo& RequestedActionInfo);

    // Assigning default values to the local WebProviders
    // and pass them later to the  WordSearchService->SearchWordOnline(Word, DefinitionProvider, TranslationProvider);
    EEVWebProvider ActiveDefinitionUsageProvider = EEVWebProvider::FreeDictionary;
    EEVWebProvider ActiveTranslationProvider = EEVWebProvider::MyMemory;
};
