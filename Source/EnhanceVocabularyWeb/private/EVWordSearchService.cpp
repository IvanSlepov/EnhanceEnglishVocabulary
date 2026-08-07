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
                                            EEVWebProvider TranslationProvider,
                                            const EEVVocabularyDBContext DatabaseContext,
                                            const TArray<EEVVocabularyTranslationLanguage>& TranslationLanguages)
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

    TArray<EEVVocabularyTranslationLanguage> NormalizedTargets;
    for (const EEVVocabularyTranslationLanguage Language : TranslationLanguages)
    {
        if (Language == EEVVocabularyTranslationLanguage::None ||
            EVVocabularyLanguage::IsTranslationEquivalentToDatabaseContext(Language, DatabaseContext) ||
            NormalizedTargets.Contains(Language))
        {
            continue;
        }
        NormalizedTargets.Add(Language);
    }

    PendingTranslationRequestCount = NormalizedTargets.Num();

    SendDictionaryRequest(Word, DefinitionUsageProvider);

    for (const EEVVocabularyTranslationLanguage Language : NormalizedTargets)
    {
        SendTranslationRequest(Word, DatabaseContext, Language, TranslationProvider);
    }

    // Zero selected translations is an intentional and valid state. The dictionary callback
    // remains the completion trigger in that case.
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

void UEVWordSearchService::SendTranslationRequest(const FString& Word, const EEVVocabularyDBContext DatabaseContext,
                                                  const EEVVocabularyTranslationLanguage TranslateTo,
                                                  EEVWebProvider TranslationProvider)
{
    FEVWebProviderUrlBuildContext Context;
    Context.Word = Word;
    Context.SourceLanguage = EVVocabularyLanguage::GetDatabaseContextWebLanguageCode(DatabaseContext);
    Context.TargetLanguage = EVVocabularyLanguage::GetTranslationWebLanguageCode(TranslateTo);

    FString Url;

    if (!FEVWebProviderUrlBuilder::BuildRequestUrl(TranslationProvider, Context, Url))
    {
        PendingTranslationRequestCount = FMath::Max(0, PendingTranslationRequestCount - 1);
        TryCompleteSearch();
        return;
    }

    HttpService->SendGetRequest(
        Url, FEVHttpResponseDelegate::CreateUObject(this, &ThisClass::HandleTranslationResponse, TranslateTo));

    UE_LOG(LogTemp, Warning, TEXT("Translation request sent: %s"), *Url);
}

void UEVWordSearchService::HandleDictionaryResponse(const bool bSuccess, const int32 ResponseCode,
                                                    const FString& ResponseBody)
{
    bDictionaryCompleted = true;
    bDictionarySucceeded = false;

    if (!bSuccess || ResponseCode != 200)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dictionary request failed. Response code: %d"), ResponseCode);

        TryCompleteSearch();
        return;
    }

    FEVVocabularyRecord ParsedDictionaryRecord;

    if (!FEVResponseParser::ParseFreeDictionaryResponse(ResponseBody, ParsedDictionaryRecord))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to parse FreeDictionary response."));

        TryCompleteSearch();
        return;
    }

    //
    // Translation requests run independently from the dictionary request.
    //
    // Some translations may already have completed by the time the
    // dictionary response arrives. ParseFreeDictionaryResponse() resets
    // its output record, so preserve those translations before replacing
    // PendingRecord with the parsed dictionary data.
    //
    TArray<FEVVocabularyTranslation> CompletedTranslations = MoveTemp(PendingRecord.GeneralTranslations);

    PendingRecord = MoveTemp(ParsedDictionaryRecord);

    PendingRecord.GeneralTranslations = MoveTemp(CompletedTranslations);

    //
    // Keep the normalized search identity established when the request
    // started.
    //
    if (PendingRecord.Word.IsEmpty())
    {
        PendingRecord.Word = CurrentSearchWord;
    }

    PendingRecord.NormalizedWord = CurrentSearchWord;

    bDictionarySucceeded = true;

    TryCompleteSearch();
}

void UEVWordSearchService::HandleTranslationResponse(const bool bSuccess, const int32 ResponseCode,
                                                     const FString& ResponseBody,
                                                     const EEVVocabularyTranslationLanguage TargetLanguage)
{
    if (bSuccess && ResponseCode == 200)
    {
        TArray<FEVVocabularyTranslation> Translations;
        const FString StorageCode = EVVocabularyLanguage::GetTranslationStorageCode(TargetLanguage);
        if (!StorageCode.IsEmpty() &&
            FEVResponseParser::ParseMyMemoryTranslationResponse(ResponseBody, StorageCode, Translations))
        {
            for (FEVVocabularyTranslation& Translation : Translations)
            {
                const bool bDuplicate = PendingRecord.GeneralTranslations.ContainsByPredicate(
                    [&Translation](const FEVVocabularyTranslation& Existing)
                    {
                        return Existing.TargetLanguage.Equals(Translation.TargetLanguage, ESearchCase::IgnoreCase) &&
                               Existing.TranslationText.Equals(Translation.TranslationText, ESearchCase::IgnoreCase);
                    });
                if (bDuplicate)
                {
                    continue;
                }

                Translation.DisplayOrder = PendingRecord.GeneralTranslations.Num();
                PendingRecord.GeneralTranslations.Add(MoveTemp(Translation));
            }
        }
    }

    PendingTranslationRequestCount = FMath::Max(0, PendingTranslationRequestCount - 1);
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
    PendingTranslationRequestCount = 0;
}

void UEVWordSearchService::TryCompleteSearch()
{
    if (!bDictionaryCompleted || PendingTranslationRequestCount > 0)
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
