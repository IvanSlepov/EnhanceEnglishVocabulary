// Fill out your copyright notice in the Description page of Project Settings.

#include "EVSearchResultsPanel.h"

#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "EVEntryItem.h"
#include "EVVocabularyUiStyle.h"

void UEVSearchResultsPanel::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_Save)
    {
        Button_Save->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleSaveClicked);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_Save is nullptr in EVSearchResultsPanel."));
    }

    if (Button_Discard)
    {
        Button_Discard->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleDiscardClicked);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_Discard is nullptr in EVSearchResultsPanel."));
    }

    if (!TextBlock_SearchResultsTranscription_Value)
    {
        UE_LOG(LogTemp, Error,
               TEXT("TextBlock_SearchResultsTranscription_Value is nullptr "
                    "in EVSearchResultsPanel."));
    }

    if (!TextBlock_SearchResultsPronunciation_Value)
    {
        UE_LOG(LogTemp, Error,
               TEXT("TextBlock_SearchResultsPronunciation_Value is nullptr "
                    "in EVSearchResultsPanel."));
    }

    if (!ListView_SearchResults)
    {
        UE_LOG(LogTemp, Error, TEXT("ListView_SearchResults is nullptr in EVSearchResultsPanel."));
    }
}

void UEVSearchResultsPanel::NativeConstruct()
{
    Super::NativeConstruct();

    if (Button_Save)
    {
        Button_Save->SetIsEnabled(true);
    }

    if (Button_Discard)
    {
        Button_Discard->SetIsEnabled(true);
    }
}

void UEVSearchResultsPanel::SetSearchResult(const FWordSearchResult& InSearchResult)
{
    CurrentVocabularyRecord = InSearchResult.VocabularyRecord;

    PopulatePronunciationFields();
    PopulateMeaningList();
}

void UEVSearchResultsPanel::ClearSearchResult()
{
    CurrentVocabularyRecord = FEVVocabularyRecord{};

    if (TextBlock_SearchResultsTranscription_Value)
    {
        TextBlock_SearchResultsTranscription_Value->SetText(FText::GetEmpty());

        TextBlock_SearchResultsTranscription_Value->SetColorAndOpacity(
            EVVocabularyUiStyle::GetNormalWrodEntryTextFontColor());
    }

    if (TextBlock_SearchResultsPronunciation_Value)
    {
        TextBlock_SearchResultsPronunciation_Value->SetText(FText::GetEmpty());

        TextBlock_SearchResultsPronunciation_Value->SetColorAndOpacity(
            EVVocabularyUiStyle::GetNormalWrodEntryTextFontColor());
    }

    if (ListView_SearchResults)
    {
        ListView_SearchResults->ClearListItems();
    }
}

const FEVVocabularyRecord& UEVSearchResultsPanel::GetCurrentVocabularyRecord() const
{
    return CurrentVocabularyRecord;
}

void UEVSearchResultsPanel::HandleSaveClicked()
{
    OnSaveClicked.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("Save pressed in EVSearchResultsPanel."));
}

void UEVSearchResultsPanel::HandleDiscardClicked()
{
    OnDiscardClicked.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("Discard pressed in EVSearchResultsPanel."));
}

void UEVSearchResultsPanel::PopulatePronunciationFields()
{
    if (!TextBlock_SearchResultsTranscription_Value || !TextBlock_SearchResultsPronunciation_Value)
    {
        UE_LOG(LogTemp, Error,
               TEXT("Cannot populate pronunciation fields because one or more "
                    "bound TextBlocks are nullptr in EVSearchResultsPanel."));

        return;
    }

    const FEVVocabularyPronunciation* PrimaryPronunciation = ResolvePrimaryPronunciation();

    const bool bHasTranscription = PrimaryPronunciation && !PrimaryPronunciation->Transcription.IsEmpty();

    TextBlock_SearchResultsTranscription_Value->SetText(FText::FromString(
        bHasTranscription ? PrimaryPronunciation->Transcription : TEXT("No transcription was provided")));

    TextBlock_SearchResultsTranscription_Value->SetColorAndOpacity(
        bHasTranscription ? EVVocabularyUiStyle::GetNormalTranscriptionTextFontColor()
                          : EVVocabularyUiStyle::GetMissingWrodEntryTextFontColor());

    const bool bHasPronunciationAudio = PrimaryPronunciation && !PrimaryPronunciation->AudioUrl.IsEmpty();

    TextBlock_SearchResultsPronunciation_Value->SetText(FText::FromString(
        bHasPronunciationAudio ? PrimaryPronunciation->AudioUrl : TEXT("No pronunciation was provided")));

    TextBlock_SearchResultsPronunciation_Value->SetColorAndOpacity(
        bHasPronunciationAudio ? EVVocabularyUiStyle::GetNormalWrodEntryTextFontColor()
                               : EVVocabularyUiStyle::GetMissingWrodEntryTextFontColor());
}

void UEVSearchResultsPanel::PopulateMeaningList()
{
    if (!ListView_SearchResults)
    {
        UE_LOG(LogTemp, Error,
               TEXT("Cannot populate meanings because ListView_SearchResults "
                    "is nullptr in EVSearchResultsPanel."));

        return;
    }

    ListView_SearchResults->ClearListItems();

    for (int32 MeaningIndex = 0; MeaningIndex < CurrentVocabularyRecord.Meanings.Num(); ++MeaningIndex)
    {
        const FEVVocabularyMeaning& SourceMeaning = CurrentVocabularyRecord.Meanings[MeaningIndex];

        UEVEntryItem* MeaningItem = NewObject<UEVEntryItem>(this);

        if (!MeaningItem)
        {
            UE_LOG(LogTemp, Error,
                   TEXT("Failed to create UEVEntryItem for a search-result "
                        "meaning in EVSearchResultsPanel."));

            continue;
        }

        MeaningItem->PayloadType = EEVEntryItemPayloadType::VocabularyMeaning;

        MeaningItem->VocabularyMeaning = BuildMeaningForDisplay(SourceMeaning, MeaningIndex == 0);

        ListView_SearchResults->AddItem(MeaningItem);
    }

    if (CurrentVocabularyRecord.Meanings.IsEmpty())
    {
        UE_LOG(LogTemp, Warning,
               TEXT("The current vocabulary record contains no meanings in "
                    "EVSearchResultsPanel."));
    }
}

const FEVVocabularyPronunciation* UEVSearchResultsPanel::ResolvePrimaryPronunciation() const
{
    const FString SelectedLanguageCode = ResolveSelectedVocabularyLanguageCode();

    for (const FEVVocabularyPronunciation& Pronunciation : CurrentVocabularyRecord.Pronunciations)
    {
        if (Pronunciation.LanguageCode.Equals(SelectedLanguageCode, ESearchCase::IgnoreCase) && Pronunciation.bPrimary)
        {
            return &Pronunciation;
        }
    }

    for (const FEVVocabularyPronunciation& Pronunciation : CurrentVocabularyRecord.Pronunciations)
    {
        if (Pronunciation.LanguageCode.Equals(SelectedLanguageCode, ESearchCase::IgnoreCase))
        {
            return &Pronunciation;
        }
    }

    return nullptr;
}

FString UEVSearchResultsPanel::ResolveSelectedVocabularyLanguageCode() const
{
    return TEXT("en");
}

FString UEVSearchResultsPanel::ResolveSelectedTranslationLanguageCode() const
{
    return TEXT("uk");
}

FEVVocabularyMeaning UEVSearchResultsPanel::BuildMeaningForDisplay(const FEVVocabularyMeaning& SourceMeaning,
                                                                   const bool bIsFirstMeaning) const
{
    FEVVocabularyMeaning Result = SourceMeaning;

    const FString SelectedLanguageCode = ResolveSelectedTranslationLanguageCode();

    Result.Translations.RemoveAll(
        [&SelectedLanguageCode](const FEVVocabularyTranslation& Translation)
        { return !Translation.TargetLanguage.Equals(SelectedLanguageCode, ESearchCase::IgnoreCase); });

    if (!Result.Translations.IsEmpty() || !bIsFirstMeaning)
    {
        return Result;
    }

    for (const FEVVocabularyTranslation& GeneralTranslation : CurrentVocabularyRecord.GeneralTranslations)
    {
        if (!GeneralTranslation.TargetLanguage.Equals(SelectedLanguageCode, ESearchCase::IgnoreCase))
        {
            continue;
        }

        if (GeneralTranslation.TranslationText.IsEmpty())
        {
            continue;
        }

        const bool bAlreadyAdded = Result.Translations.ContainsByPredicate(
            [&GeneralTranslation](const FEVVocabularyTranslation& ExistingTranslation)
            {
                return ExistingTranslation.TargetLanguage.Equals(GeneralTranslation.TargetLanguage,
                                                                 ESearchCase::IgnoreCase) &&
                       ExistingTranslation.TranslationText.Equals(GeneralTranslation.TranslationText,
                                                                  ESearchCase::IgnoreCase);
            });

        if (!bAlreadyAdded)
        {
            Result.Translations.Add(GeneralTranslation);
        }
    }

    return Result;
}