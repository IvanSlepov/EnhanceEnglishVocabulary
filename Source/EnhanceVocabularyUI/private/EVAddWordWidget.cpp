// Fill out your copyright notice in the Description page of Project Settings.

#include "EVAddWordWidget.h"
#include "EVSearchResultsPanel.h"
#include "EVWordInputValidator.h"
#include "EVErrorTypes.h"
#include "EVRequestedActionTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EVVocabularyUiStyle.h"

namespace
{
FName GetWebProviderId(const EEVWebProvider Provider)
{
    switch (Provider)
    {
    case EEVWebProvider::FreeDictionary:
        return FName(TEXT("FreeDictionary"));
    case EEVWebProvider::MyMemory:
        return FName(TEXT("MyMemory"));
    case EEVWebProvider::Datamuse:
        return FName(TEXT("Datamuse"));
    default:
        return NAME_None;
    }
}
} // namespace

void UEVAddWordWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

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

    if (WBP_SearchResultsPanel)
    {
        WBP_SearchResultsPanel->SetVisibility(ESlateVisibility::Hidden);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("WBP_SearchResultsPanel is nullptr"));
    }
    Button_Search->SetIsEnabled(false);
    Button_Clear->SetIsEnabled(false);
}

void UEVAddWordWidget::SetWordInput(const FString& Word)
{
    if (!EditableText_WordInput)
    {
        return;
    }

    EnableEditableTextBox(true);
    EditableText_WordInput->SetText(FText::FromString(Word));
    EditableText_WordInput->SetKeyboardFocus();
}

void UEVAddWordWidget::EnableEditableTextBox(bool bEnable)
{
    if (EditableText_WordInput)
    {
        EditableText_WordInput->SetIsReadOnly(!bEnable);
    }

    if (Button_Search)
    {
        Button_Search->SetIsEnabled(bEnable);
    }

    if (Button_Clear)
    {
        Button_Clear->SetIsEnabled(bEnable);
    }

    // Any future controls that depend on the editable state
    // should also be handled here.
}

void UEVAddWordWidget::SetControlsEnabled(bool bEnabled)
{
    bAreInteractionElementsEnabled = bEnabled;
}

bool UEVAddWordWidget::GetControlsEnabled()
{
    return bAreInteractionElementsEnabled;
}

void UEVAddWordWidget::SetInputEnabled(bool bSetInputEnabled)
{
    EnableEditableTextBox(bSetInputEnabled);
}

void UEVAddWordWidget::ClearStoredSearchResultVariable(FWordSearchResult& CachedWordSearchResult)
{
    CachedWordSearchResult = FWordSearchResult{};
}

void UEVAddWordWidget::HandleOnSearchPressed()
{
    if (!bAreInteractionElementsEnabled)
    {
        HandleOnWidgetInteractionDisabled();
        return;
    }

    ClearStoredSearchResultVariable(WordSearchResult);

    if (WBP_SearchResultsPanel)
    {
        WBP_SearchResultsPanel->ClearSearchResult();
        WBP_SearchResultsPanel->SetVisibility(ESlateVisibility::Hidden);
    }

    EnableEditableTextBox(false);

    if (!OnVocabularySearchRequested.IsBound())
    {
        UE_LOG(LogTemp, Error, TEXT("Vocabulary search application port in EVAddWordWidget.cpp is unavailable"));

        EnableEditableTextBox(true);
        return;
    }

    FString NormalizedWord;
    FText WordInputError;
    FEVErrorInfo EVErrorInfo;

    const FString WordToSearch = EditableText_WordInput->GetText().ToString();

    switch (FEVWordInputValidator::ValidateSearchInput(WordToSearch, NormalizedWord, WordInputError))
    {
    case EEVInputValidationResult::Valid:
    {
        FEVVocabularySearchRequest Request;
        Request.RequestId = FGuid::NewGuid();
        Request.Word = NormalizedWord;
        Request.DefinitionProviderId = GetWebProviderId(ActiveDefinitionUsageProvider);
        Request.TranslationProviderId = GetWebProviderId(ActiveTranslationProvider);

        PendingVocabularySearchRequestId = Request.RequestId;
        HandleOnLoadingDataTriggerred(true);

        OnVocabularySearchRequested.Broadcast(Request);

        break;
    }

    case EEVInputValidationResult::EmptyInput:
        EVErrorInfo.Source = EEVErrorSource::AddWord;
        EVErrorInfo.Type = EEVErrorType::EmptyString;
        EVErrorInfo.Message = WordInputError;

        EnableEditableTextBox(true);
        OnError.Broadcast(EVErrorInfo);
        OnFeatureError.Broadcast(EVErrorInfo);

        break;

    case EEVInputValidationResult::InvalidCharacters:
        EVErrorInfo.Source = EEVErrorSource::AddWord;
        EVErrorInfo.Type = EEVErrorType::InvalidInput;
        EVErrorInfo.Message = WordInputError;

        EnableEditableTextBox(true);
        OnError.Broadcast(EVErrorInfo);
        OnFeatureError.Broadcast(EVErrorInfo);

        break;

    default:
        EnableEditableTextBox(true);
        break;
    }
}

void UEVAddWordWidget::ApplyVocabularySearchOutcome(const FEVVocabularySearchOutcome& Outcome)
{
    if (!PendingVocabularySearchRequestId.IsValid() || Outcome.RequestId != PendingVocabularySearchRequestId)
    {
        return;
    }

    PendingVocabularySearchRequestId = FGuid();

    FWordSearchResult Result;
    Result.Word = Outcome.Record.Word;
    Result.NormalizedWord = Outcome.Record.NormalizedWord;
    Result.bSuccess = Outcome.Result == EEVApplicationOperationResult::Succeeded;
    Result.ErrorMessage = Outcome.Message.ToString();
    Result.VocabularyRecord = Outcome.Record;

    HandleSearchWordCompleted(Result);
}

void UEVAddWordWidget::HandleSearchWordCompleted(const FWordSearchResult& Result)
{
    HandleOnLoadingDataTriggerred(false);

    if (!Result.bSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("Word search failed."));

        ClearStoredSearchResultVariable(WordSearchResult);

        if (WBP_SearchResultsPanel)
        {
            WBP_SearchResultsPanel->ClearSearchResult();
            WBP_SearchResultsPanel->SetVisibility(ESlateVisibility::Hidden);
        }

        FEVErrorInfo EVErrorInfo;
        EVErrorInfo.Source = EEVErrorSource::AddWord;
        EVErrorInfo.Type = EEVErrorType::SearchError;
        EVErrorInfo.Message = FText::FromString(TEXT("We couldn't find that word. Please check the spelling "
                                                     "or change your dictionary provider in settings."));

        EnableEditableTextBox(false);
        OnError.Broadcast(EVErrorInfo);
        OnFeatureError.Broadcast(EVErrorInfo);

        return;
    }

    WordSearchResult = Result;

    Button_Search->SetIsEnabled(false);
    Button_Clear->SetIsEnabled(false);

    if (!WBP_SearchResultsPanel)
    {
        UE_LOG(LogTemp, Error,
               TEXT("WBP_SearchResultsPanel is nullptr in "
                    "HandleSearchWordCompleted."));

        EnableEditableTextBox(true);
        return;
    }

    WBP_SearchResultsPanel->SetSearchResult(Result);
    WBP_SearchResultsPanel->SetVisibility(ESlateVisibility::Visible);
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

void UEVAddWordWidget::HandleOnWidgetInteractionDisabled()
{
    OnWidgetInteractionDisabled.Broadcast(bAreInteractionElementsEnabled, ThisWidgetName);
}

void UEVAddWordWidget::HandleOnLoadingDataTriggerred(bool ShowLoadingSpinner)
{
    OnLoadingDataTriggerred.Broadcast(ShowLoadingSpinner);
}

void UEVAddWordWidget::HandleOnActionRequested(const FEVRequestedActionInfo& RequestedActionInfo)
{
    OnActionRequested.Broadcast(RequestedActionInfo);
}

void UEVAddWordWidget::HandleOnSaveSearchResultPressed()
{
    if (!bAreInteractionElementsEnabled)
    {
        HandleOnWidgetInteractionDisabled();
        return;
    }

    if (!OnVocabularyRecordRequested.IsBound())
    {
        UE_LOG(LogTemp, Error, TEXT("Vocabulary library application port in EVAddWordWidget.cpp is unavailable"));
        return;
    }

    if (PendingVocabularyRecordRequestId.IsValid() || PendingVocabularyMutationRequestId.IsValid())
    {
        return;
    }

    FEVVocabularyRecordRequest Request;
    Request.RequestId = FGuid::NewGuid();
    Request.Word = WordSearchResult.Word;

    PendingVocabularyRecordRequestId = Request.RequestId;
    OnVocabularyRecordRequested.Broadcast(Request);
}

void UEVAddWordWidget::ApplyVocabularyRecordOutcome(const FEVVocabularyRecordOutcome& Outcome)
{
    if (!PendingVocabularyRecordRequestId.IsValid() || Outcome.RequestId != PendingVocabularyRecordRequestId)
    {
        return;
    }

    PendingVocabularyRecordRequestId = FGuid();

    if (Outcome.bExists)
    {
        FEVErrorInfo EVErrorInfo;
        EVErrorInfo.Source = EEVErrorSource::Database;
        EVErrorInfo.Type = EEVErrorType::DuplicateWord;
        EVErrorInfo.Message = Outcome.Message;

        Button_Search->SetIsEnabled(true);
        Button_Clear->SetIsEnabled(true);

        EnableEditableTextBox(true);

        if (WBP_SearchResultsPanel)
        {
            WBP_SearchResultsPanel->ClearSearchResult();
            WBP_SearchResultsPanel->SetVisibility(ESlateVisibility::Hidden);
        }

        ClearStoredSearchResultVariable(WordSearchResult);

        OnError.Broadcast(EVErrorInfo);
        OnFeatureError.Broadcast(EVErrorInfo);
        return;
    }

    if (!OnVocabularyMutationRequested.IsBound())
    {
        UE_LOG(LogTemp, Error, TEXT("Vocabulary mutation application port in EVAddWordWidget.cpp is unavailable"));
        return;
    }

    FEVVocabularyMutationRequest Request;
    Request.RequestId = FGuid::NewGuid();
    Request.MutationType = EEVVocabularyMutationType::Save;
    Request.Record = WordSearchResult.VocabularyRecord;

    PendingVocabularyMutationRequestId = Request.RequestId;
    OnVocabularyMutationRequested.Broadcast(Request);
}

void UEVAddWordWidget::ApplyVocabularyMutationOutcome(const FEVVocabularyMutationOutcome& Outcome)
{
    if (!PendingVocabularyMutationRequestId.IsValid() || Outcome.RequestId != PendingVocabularyMutationRequestId)
    {
        return;
    }

    PendingVocabularyMutationRequestId = FGuid();

    if (Outcome.Result != EEVApplicationOperationResult::Succeeded)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot save the entry in WBP_AddWord."));
        return;
    }

    FEVRequestedActionInfo EVRequestedActionInfo;

    EVRequestedActionInfo.Source = EEVRequestedActionSource::AddWord;

    EVRequestedActionInfo.Type = EEVRequestedActionType::SaveWord;

    EVRequestedActionInfo.Status = EEVRequestedActionStatus::Saved;

    EVRequestedActionInfo.GenerateMessage();
    EVRequestedActionInfo.GenerateColor();

    HandleOnActionRequested(EVRequestedActionInfo);

    Button_Search->SetIsEnabled(false);
    Button_Clear->SetIsEnabled(false);

    EnableEditableTextBox(true);

    EditableText_WordInput->SetText(FText::GetEmpty());

    if (WBP_SearchResultsPanel)
    {
        WBP_SearchResultsPanel->ClearSearchResult();
        WBP_SearchResultsPanel->SetVisibility(ESlateVisibility::Hidden);
    }

    ClearStoredSearchResultVariable(WordSearchResult);
}

void UEVAddWordWidget::HandleOnDiscardSearchResultPressed()
{
    if (!bAreInteractionElementsEnabled)
    {
        HandleOnWidgetInteractionDisabled();
        return;
    }

    EnableEditableTextBox(true);

    Button_Search->SetIsEnabled(true);
    Button_Clear->SetIsEnabled(true);

    if (WBP_SearchResultsPanel)
    {
        WBP_SearchResultsPanel->ClearSearchResult();
        WBP_SearchResultsPanel->SetVisibility(ESlateVisibility::Hidden);
    }

    ClearStoredSearchResultVariable(WordSearchResult);
}

void UEVAddWordWidget::HandleWebProvidersChanged(EEVWebProvider DefinitionUsageProvider,
                                                 EEVWebProvider TranslationProvider)
{
    ActiveDefinitionUsageProvider = DefinitionUsageProvider;
    ActiveTranslationProvider = TranslationProvider;
}

void UEVAddWordWidget::ApplyFeatureErrorResolution(const FEVErrorInfo& ErrorInfo)
{
    if (ErrorInfo.Source == EEVErrorSource::AddWord && ErrorInfo.Type == EEVErrorType::SearchError)
    {
        SetInputEnabled(true);
    }
}

void UEVAddWordWidget::ApplyNetworkConnectivityState(const EEVApplicationConnectivityState State)
{
    SetControlsEnabled(State == EEVApplicationConnectivityState::Online);
}

void UEVAddWordWidget::ApplyWebProviderSelection(const EEVWebProvider DefinitionProvider,
                                                 const EEVWebProvider TranslationProvider)
{
    HandleWebProvidersChanged(DefinitionProvider, TranslationProvider);
}

void UEVAddWordWidget::ApplyVocabularyPreferences(const FEVVocabularyLanguagePreferences& Preferences)
{
    if (WBP_SearchResultsPanel)
    {
        WBP_SearchResultsPanel->ApplyVocabularyPreferences(Preferences);
    }
}

void UEVAddWordWidget::PresentWordContext(const FString& Word)
{
    SetWordInput(Word);
}
