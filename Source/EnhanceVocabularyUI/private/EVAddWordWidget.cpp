// Fill out your copyright notice in the Description page of Project Settings.

#include "EVAddWordWidget.h"
#include "EVSearchResultsPanel.h"
#include "EnhanceVocabulary/EVGameInstance.h"
#include "EnhanceVocabularyStorage/public/EVEntryItem.h"
#include "EVWordInputValidator.h"
#include "EVErrorTypes.h"
#include "Kismet/KismetSystemLibrary.h"

void UEVAddWordWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    EVGameInstance = Cast<UEVGameInstance>(GetGameInstance());

    if (EVGameInstance)
    {
        EVGameInstance->OnEVWordSearchCompletedFromEVGameInstance.AddDynamic(this,
                                                                             &ThisClass::HandleSearchWordCompleted);
    }

    else
    {
        UE_LOG(LogTemp, Error, TEXT("EVGameInstance is null"));
    }

    if (WBP_SearchResultsPanel && Button_Search && Button_Clear && EditableText_WordInput)
    {
        WBP_SearchResultsPanel->OnSaveClicked.AddDynamic(this, &ThisClass::HandleOnSaveSearchResultPressed);
        WBP_SearchResultsPanel->OnDiscardClicked.AddDynamic(this, &ThisClass::HandleOnDiscardSearchResultPressed);
        Button_Search->OnPressed.AddDynamic(this, &ThisClass::HandleOnSearchPressed);
        Button_Clear->OnPressed.AddDynamic(this, &ThisClass::HandleOnClearPressed);
        EditableText_WordInput->OnTextChanged.AddDynamic(this, &ThisClass::HandleEditableTextBoxTextChanged);
    }

    else
    {
        UE_LOG(LogTemp, Error, TEXT("The AddWord bound widgets are nullptr"));
    }
}

void UEVAddWordWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UEVAddWordWidget::NativeConstruct()
{
    Super::NativeConstruct();

    Init();
}

void UEVAddWordWidget::Init()
{
    ThisWidgetName = GetName();

    if (!WBP_SearchResultsPanel)
    {
        UE_LOG(LogTemp, Error, TEXT("WBP_SearchResultsPanel is nullptr"));
    }

    WBP_SearchResultsPanel->SetVisibility(ESlateVisibility::Hidden);
    Button_Search->SetIsEnabled(false);
    Button_Clear->SetIsEnabled(false);
}

void UEVAddWordWidget::ClearStoredSearchResultVariable(FWordSearchResult& CachedWordSearchResult)
{
    CachedWordSearchResult = FWordSearchResult{};
}

void UEVAddWordWidget::HandleOnSearchPressed()
{
    // Call the event in case local interaction has been disabled
    if (!bAreInteractionElementsEnabled)
    {
        HandleOnWidgetInteractionDisabled();
        return;
    }

    // We need to clear the local WordSearchResult struct to avoid setting
    // WBP_SearchResultPanel Visible in case of the bad input/existing word
    ClearStoredSearchResultVariable(WordSearchResult);

    if (!EVGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("EVGameInstance in EVAddWordWidget.cpp is nullptr"));
    }

    FString NormalizedWord;
    FText WordInputError;
    FEVErrorInfo EVErrorInfo;
    FString WordToSearch = EditableText_WordInput->GetText().ToString();

    switch (FEVWordInputValidator::ValidateSearchInput(WordToSearch, NormalizedWord, WordInputError))
    {
    case EEVInputValidationResult::Valid:
        // WordSearchResult = EVGameInstance->SearchWordFake(NormalizedWord); - leave this for debugging purposes
        EVGameInstance->SearchWordOnline(NormalizedWord);
        break;
    case EEVInputValidationResult::EmptyInput:
        EVErrorInfo.Source = EEVErrorSource::AddWord;
        EVErrorInfo.Type = EEVErrorType::EmptyString;
        EVErrorInfo.Message = WordInputError;
        OnError.Broadcast(EVErrorInfo);
        break;
    case EEVInputValidationResult::InvalidCharacters:
        EVErrorInfo.Source = EEVErrorSource::AddWord;
        EVErrorInfo.Type = EEVErrorType::InvalidInput;
        EVErrorInfo.Message = WordInputError;
        OnError.Broadcast(EVErrorInfo);
        break;
    default:
        break;
    }
}

void UEVAddWordWidget::HandleOnClearPressed()
{
    // Call the event in case local interaction has been disabled
    if (!bAreInteractionElementsEnabled)
    {
        HandleOnWidgetInteractionDisabled();
        return;
    }

    Button_Search->SetIsEnabled(false);
    Button_Clear->SetIsEnabled(false);
    EditableText_WordInput->SetText(FText::GetEmpty());
}

void UEVAddWordWidget::HandleEditableTextBoxTextChanged(const FText& NewText)
{
    // Call the event in case local interaction has been disabled
    if (!bAreInteractionElementsEnabled)
    {
        HandleOnWidgetInteractionDisabled();
        return;
    }

    Button_Search->SetIsEnabled(true);
    Button_Clear->SetIsEnabled(true);
}

void UEVAddWordWidget::HandleOnSaveSearchResultPressed()
{
    // Call the event in case local interaction has been disabled
    if (!bAreInteractionElementsEnabled)
    {
        HandleOnWidgetInteractionDisabled();
        return;
    }

    if (!EVGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("EVGameInstance in EVAddWordWidget.cpp is nullptr"));
    }

    FEVErrorInfo EVErrorInfo;
    FText OutErrorMessage;

    if (EVGameInstance->DoesWordExist(WordSearchResult.Word, OutErrorMessage) ==
        EEVVocabularyStorageServiceResult::WordExists)
    {

        EVErrorInfo.Source = EEVErrorSource::Database;
        EVErrorInfo.Type = EEVErrorType::DuplicateWord;
        EVErrorInfo.Message = OutErrorMessage;

        EditableText_WordInput->SetText(FText::GetEmpty());
        OnError.Broadcast(EVErrorInfo);
    }

    if (EVGameInstance->SaveVocabularyEntry(WordSearchResult))
    {
        Button_Search->SetIsEnabled(false);
        Button_Clear->SetIsEnabled(false);
        EditableText_WordInput->SetText(FText::GetEmpty());
        WBP_SearchResultsPanel->TextBlock_SearchResultsDefinition->SetText(FText::GetEmpty());
        WBP_SearchResultsPanel->TextBlock_SearchResultsUsage->SetText(FText::GetEmpty());
        WBP_SearchResultsPanel->TextBlock_SearchResultsTranslation_Russian->SetText(FText::GetEmpty());
        WBP_SearchResultsPanel->TextBlock_SearchResultsTranslation_Ukrainian->SetText(FText::GetEmpty());
        WBP_SearchResultsPanel->SetVisibility(ESlateVisibility::Hidden);
    }

    else
    {
        UE_LOG(LogTemp, Error, TEXT("Can not save the entry in WBP_AddWord!!!"));
    }
}

void UEVAddWordWidget::HandleOnDiscardSearchResultPressed()
{
    // Call the event in case local interaction has been disabled
    if (!bAreInteractionElementsEnabled)
    {
        HandleOnWidgetInteractionDisabled();
        return;
    }

    Button_Search->SetIsEnabled(true);
    Button_Clear->SetIsEnabled(true);
    WBP_SearchResultsPanel->SetVisibility(ESlateVisibility::Hidden);
}

void UEVAddWordWidget::SetControlsEnabled(bool bEnabled)
{
    bAreInteractionElementsEnabled = bEnabled;
}

bool UEVAddWordWidget::GetControlsEnabled()
{
    return bAreInteractionElementsEnabled;
}

void UEVAddWordWidget::HandleOnWidgetInteractionDisabled()
{
    OnWidgetInteractionDisabled.Broadcast(bAreInteractionElementsEnabled, ThisWidgetName);
}

void UEVAddWordWidget::HandleSearchWordCompleted(const FWordSearchResult& Result)
{
    if (Result.bSuccess)
    {
        WordSearchResult = Result;

        Button_Search->SetIsEnabled(false);
        Button_Clear->SetIsEnabled(false);

        WBP_SearchResultsPanel->SetVisibility(ESlateVisibility::Visible);

        WBP_SearchResultsPanel->TextBlock_SearchResultsDefinition->SetText(FText::FromString(Result.Definition));

        WBP_SearchResultsPanel->TextBlock_SearchResultsUsage->SetText(FText::FromString(Result.Usage));

        WBP_SearchResultsPanel->TextBlock_SearchResultsTranslation_Russian->SetText(
            FText::FromString(Result.TranslationRu));

        WBP_SearchResultsPanel->TextBlock_SearchResultsTranslation_Ukrainian->SetText(
            FText::FromString(Result.TranslationUa));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Word search failed."));

        FEVErrorInfo EVErrorInfo;
        EVErrorInfo.Source = EEVErrorSource::AddWord;
        EVErrorInfo.Type = EEVErrorType::SearchError;
        EVErrorInfo.Message = FText::FromString(TEXT(
            "We couldn't find that word. Please check the spelling or change your dictionary provider in settings."));
        OnError.Broadcast(EVErrorInfo);
    }
}