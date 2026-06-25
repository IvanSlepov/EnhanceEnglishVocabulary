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
    virtual FOnWidgetInteractionDisabled& GetWidgetInteractionDisabledEvent() override
    {
        return OnWidgetInteractionDisabled;
    }

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    void Init();
    void ClearStoredSearchResultVariable(FWordSearchResult& CachedWordSearchResult);

    // Handle events from WBP_SearchResulsPanel
    UFUNCTION()
    void HandleOnSaveSearchResultPressed();

    UFUNCTION()
    void HandleOnDiscardSearchResultPressed();

    // Handle internal Button_Search and Button_Clear events
    UFUNCTION()
    void HandleOnSearchPressed();

    UFUNCTION()
    void HandleOnClearPressed();

    // Handle change of text in the EditableText_WordInput
    UFUNCTION()
    void HandleEditableTextBoxTextChanged(const FText& NewText);

    FWordSearchResult WordSearchResult;

    bool bAreInteractionElementsEnabled = true;

    // Common Interface derrived event
    UPROPERTY(BlueprintAssignable)
    FOnWidgetInteractionDisabled OnWidgetInteractionDisabled;

    UFUNCTION()
    void HandleOnWidgetInteractionDisabled();

    UFUNCTION()
    void HandleSearchWordCompleted(const FWordSearchResult& Result);

    FString ThisWidgetName;
};
