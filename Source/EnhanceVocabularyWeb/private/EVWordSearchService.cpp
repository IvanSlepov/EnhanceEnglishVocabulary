// Fill out your copyright notice in the Description page of Project Settings.

#include "EVWordSearchService.h"

#include "EVHttpService.h"
#include "EVResponseParser.h"
#include "EVWebProviderUrlBuilder.h"
#include "EVWordInputValidator.h"

void UEVWordSearchService::Initialize()
{
    HttpService = NewObject<UEVHttpService>(this);
}

FWordSearchResult UEVWordSearchService::SearchWordFake(const FString& Word)
{
    FWordSearchResult Result;

    const FString NormalizedWord = FEVWordInputValidator::NormalizeWordInput(Word);

    FEVVocabularyRecord Record;
    Record.Word = NormalizedWord;
    Record.NormalizedWord = NormalizedWord;

    FEVVocabularyPronunciation Pronunciation;
    Pronunciation.LanguageCode = TEXT("en");
    Pronunciation.Transcription = TEXT("/fake/");
    Pronunciation.bPrimary = true;
    Pronunciation.ProviderName = TEXT("Fake");
    Record.Pronunciations.Add(MoveTemp(Pronunciation));

    FEVVocabularyMeaning Meaning;
    Meaning.PartOfSpeech = TEXT("unspecified");
    Meaning.ProviderName = TEXT("Fake");

    FEVVocabularyDefinition Definition;
    Definition.DefinitionText = FString::Printf(TEXT("Fake definition for: %s"), *NormalizedWord);
    Definition.UsageExample = FString::Printf(TEXT("Fake usage example for: %s"), *NormalizedWord);
    Definition.ProviderName = TEXT("Fake");
    Meaning.Definitions.Add(MoveTemp(Definition));
    Record.Meanings.Add(MoveTemp(Meaning));

    FEVVocabularyTranslation TranslationRu;
    TranslationRu.TranslationText = TEXT("Fake Russian translation");
    TranslationRu.TargetLanguage = TEXT("ru");
    TranslationRu.ProviderName = TEXT("Fake");
    Record.GeneralTranslations.Add(MoveTemp(TranslationRu));

    FEVVocabularyTranslation TranslationUk;
    TranslationUk.TranslationText = TEXT("Fake Ukrainian translation");
    TranslationUk.TargetLanguage = TEXT("uk");
    TranslationUk.ProviderName = TEXT("Fake");
    Record.GeneralTranslations.Add(MoveTemp(TranslationUk));

    PendingRecord = MoveTemp(Record);
    PendingResult = FWordSearchResult();
    PendingResult.bSuccess = true;
    PopulateLegacySearchResult();
    Result = PendingResult;

    PendingRecord = FEVVocabularyRecord();
    PendingResult = FWordSearchResult();
    return Result;
}

void UEVWordSearchService::SearchWordOnline(const FString& Word, EEVWebProvider DefinitionUsageProvider,
                                            EEVWebProvider TranslationProvider)
{
    if (!HttpService)
    {
        UE_LOG(LogTemp, Error, TEXT("HttpService is null"));
        return;
    }

    if (Word.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("SearchWordOnline: Word is empty"));
        return;
    }

    ResetPendingSearch(Word);

    SendDictionaryRequest(Word, DefinitionUsageProvider);
    SendTranslationRequest(Word, TEXT("ru"), TranslationProvider);
    SendTranslationRequest(Word, TEXT("uk"), TranslationProvider);
}

void UEVWordSearchService::SendDictionaryRequest(const FString& Word, EEVWebProvider DefinitionUsageProvider)
{
    FEVWebProviderUrlBuildContext Context;
    Context.Word = Word;

    FString Url;

    if (!FEVWebProviderUrlBuilder::BuildRequestUrl(DefinitionUsageProvider, Context, Url))
    {
        bDictionaryCompleted = true;
        bDictionarySucceeded = false;
        TryCompleteSearch();
        return;
    }

    HttpService->SendGetRequest(
        Url, FEVHttpResponseDelegate::CreateUObject(this, &UEVWordSearchService::HandleDictionaryResponse));

    UE_LOG(LogTemp, Warning, TEXT("Dictionary request sent: %s"), *Url);
}

void UEVWordSearchService::SendTranslationRequest(const FString& Word, const FString& TranslateTo,
                                                  EEVWebProvider TranslationProvider)
{
    FEVWebProviderUrlBuildContext Context;
    Context.Word = Word;
    Context.SourceLanguage = TEXT("en");
    Context.TargetLanguage = TranslateTo;

    FString Url;

    if (!FEVWebProviderUrlBuilder::BuildRequestUrl(TranslationProvider, Context, Url))
    {
        if (TranslateTo.Equals(TEXT("ru")))
        {
            bTranslationRuCompleted = true;
        }
        else if (TranslateTo.Equals(TEXT("uk")))
        {
            bTranslationUkCompleted = true;
        }

        TryCompleteSearch();
        return;
    }

    if (TranslateTo.Equals(TEXT("ru")))
    {
        HttpService->SendGetRequest(
            Url, FEVHttpResponseDelegate::CreateUObject(this, &UEVWordSearchService::HandleTranslationRuResponse));
    }
    else if (TranslateTo.Equals(TEXT("uk")))
    {
        HttpService->SendGetRequest(
            Url, FEVHttpResponseDelegate::CreateUObject(this, &UEVWordSearchService::HandleTranslationUkResponse));
    }

    UE_LOG(LogTemp, Warning, TEXT("Translation request sent: %s"), *Url);
}

void UEVWordSearchService::HandleDictionaryResponse(bool bSuccess, int32 ResponseCode, const FString& ResponseBody)
{
    bDictionaryCompleted = true;
    bDictionarySucceeded = false;

    if (bSuccess && ResponseCode == 200)
    {
        FEVVocabularyRecord ParsedRecord;
        if (FEVResponseParser::ParseFreeDictionaryResponse(ResponseBody, ParsedRecord))
        {
            PendingRecord.Word = ParsedRecord.Word;
            PendingRecord.NormalizedWord = ParsedRecord.NormalizedWord;
            PendingRecord.Pronunciations = MoveTemp(ParsedRecord.Pronunciations);
            PendingRecord.Meanings = MoveTemp(ParsedRecord.Meanings);
            bDictionarySucceeded = true;
        }
    }

    TryCompleteSearch();
}

void UEVWordSearchService::HandleTranslationRuResponse(bool bSuccess, int32 ResponseCode, const FString& ResponseBody)
{
    if (bSuccess && ResponseCode == 200)
    {
        FEVVocabularyTranslation Translation;
        if (FEVResponseParser::ParseMyMemoryTranslationResponse(ResponseBody, TEXT("ru"), Translation))
        {
            Translation.DisplayOrder = PendingRecord.GeneralTranslations.Num();
            PendingRecord.GeneralTranslations.Add(MoveTemp(Translation));
        }
    }

    bTranslationRuCompleted = true;
    TryCompleteSearch();
}

void UEVWordSearchService::HandleTranslationUkResponse(bool bSuccess, int32 ResponseCode, const FString& ResponseBody)
{
    if (bSuccess && ResponseCode == 200)
    {
        FEVVocabularyTranslation Translation;
        if (FEVResponseParser::ParseMyMemoryTranslationResponse(ResponseBody, TEXT("uk"), Translation))
        {
            Translation.DisplayOrder = PendingRecord.GeneralTranslations.Num();
            PendingRecord.GeneralTranslations.Add(MoveTemp(Translation));
        }
    }

    bTranslationUkCompleted = true;
    TryCompleteSearch();
}

void UEVWordSearchService::ResetPendingSearch(const FString& Word)
{
    PendingResult = FWordSearchResult();
    PendingRecord = FEVVocabularyRecord();

    const FString NormalizedWord = FEVWordInputValidator::NormalizeWordInput(Word);
    PendingRecord.Word = NormalizedWord;
    PendingRecord.NormalizedWord = NormalizedWord;

    CurrentSearchWord = NormalizedWord;

    bDictionaryCompleted = false;
    bDictionarySucceeded = false;
    bTranslationRuCompleted = false;
    bTranslationUkCompleted = false;
}

void UEVWordSearchService::TryCompleteSearch()
{
    if (!bDictionaryCompleted || !bTranslationRuCompleted || !bTranslationUkCompleted)
    {
        return;
    }

    PendingResult.bSuccess = bDictionarySucceeded;

    if (!bDictionarySucceeded)
    {
        PendingResult.Word = PendingRecord.Word;
        PendingResult.NormalizedWord = PendingRecord.NormalizedWord;
        PendingResult.ErrorMessage = TEXT("Dictionary provider did not return a valid vocabulary record.");
        OnEVWordSearchCompleted.Broadcast(PendingResult);
        return;
    }

    PopulateLegacySearchResult();
    OnEVWordSearchCompleted.Broadcast(PendingResult);
}

void UEVWordSearchService::PopulateLegacySearchResult()
{
    PendingResult.Word = PendingRecord.Word;
    PendingResult.NormalizedWord = PendingRecord.NormalizedWord;
    PendingResult.VocabularyRecord = PendingRecord;

    if (!PendingRecord.Pronunciations.IsEmpty())
    {
        const FEVVocabularyPronunciation* PrimaryPronunciation = PendingRecord.Pronunciations.FindByPredicate(
            [](const FEVVocabularyPronunciation& Pronunciation) { return Pronunciation.bPrimary; });

        PendingResult.Transcription =
            (PrimaryPronunciation ? PrimaryPronunciation : &PendingRecord.Pronunciations[0])->Transcription;
    }

    FString DefinitionText;
    FString UsageText;
    int32 DefinitionIndex = 1;
    bool bHasUsageExamples = false;

    for (const FEVVocabularyMeaning& Meaning : PendingRecord.Meanings)
    {
        for (const FEVVocabularyDefinition& Definition : Meaning.Definitions)
        {
            if (!Definition.DefinitionText.IsEmpty())
            {
                DefinitionText += FString::Printf(TEXT("%d. %s\n\n"), DefinitionIndex, *Definition.DefinitionText);
            }

            if (!Definition.UsageExample.IsEmpty())
            {
                UsageText += FString::Printf(TEXT("%d. %s\n\n"), DefinitionIndex, *Definition.UsageExample);
                bHasUsageExamples = true;
            }

            ++DefinitionIndex;
        }
    }

    PendingResult.Definition = DefinitionText.TrimEnd();
    PendingResult.bHasUsageExamples = bHasUsageExamples;
    PendingResult.Usage = bHasUsageExamples ? UsageText.TrimEnd() : EVVocabularyUsage::GetNoUsageExamplesText();

    TArray<FString> RussianTranslations;
    TArray<FString> UkrainianTranslations;

    for (const FEVVocabularyTranslation& Translation : PendingRecord.GeneralTranslations)
    {
        if (Translation.TargetLanguage.Equals(TEXT("ru"), ESearchCase::IgnoreCase))
        {
            RussianTranslations.Add(Translation.TranslationText);
        }
        else if (Translation.TargetLanguage.Equals(TEXT("uk"), ESearchCase::IgnoreCase) ||
                 Translation.TargetLanguage.Equals(TEXT("ua"), ESearchCase::IgnoreCase))
        {
            UkrainianTranslations.Add(Translation.TranslationText);
        }
    }

    PendingResult.TranslationRu = FString::Join(RussianTranslations, TEXT(", "));
    PendingResult.TranslationUa = FString::Join(UkrainianTranslations, TEXT(", "));
}
