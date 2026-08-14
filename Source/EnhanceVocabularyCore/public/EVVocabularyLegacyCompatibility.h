#pragma once

#include "CoreMinimal.h"
#include "EVVocabularyTypes.h"

namespace EVVocabularyLegacyCompatibility
{
inline FVocabularyEntry FlattenRecord(const FEVVocabularyRecord& Record)
{
    FVocabularyEntry Entry;
    Entry.Word = Record.Word;
    Entry.NormalizedWord = Record.NormalizedWord;

    if (!Record.Pronunciations.IsEmpty())
    {
        const FEVVocabularyPronunciation* Primary =
            Record.Pronunciations.FindByPredicate([](const FEVVocabularyPronunciation& Item) { return Item.bPrimary; });
        Entry.Transcription = (Primary ? Primary : &Record.Pronunciations[0])->Transcription;
    }

    TArray<FString> Definitions;
    TArray<FString> Usages;
    for (const FEVVocabularyMeaning& Meaning : Record.Meanings)
    {
        for (const FEVVocabularyDefinition& Definition : Meaning.Definitions)
        {
            if (!Definition.DefinitionText.IsEmpty())
            {
                Definitions.Add(Definition.DefinitionText);
            }
            if (!Definition.UsageExample.IsEmpty())
            {
                Usages.Add(Definition.UsageExample);
            }
        }
    }
    Entry.Definition = FString::Join(Definitions, TEXT("\n"));
    Entry.Usage = Usages.IsEmpty() ? EVVocabularyUsage::GetNoUsageExamplesText() : FString::Join(Usages, TEXT("\n"));

    TArray<FString> RussianTranslations;
    TArray<FString> UkrainianTranslations;
    const auto CollectTranslation =
        [&RussianTranslations, &UkrainianTranslations](const FEVVocabularyTranslation& Translation)
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
    };

    for (const FEVVocabularyTranslation& Translation : Record.GeneralTranslations)
    {
        CollectTranslation(Translation);
    }
    for (const FEVVocabularyMeaning& Meaning : Record.Meanings)
    {
        for (const FEVVocabularyTranslation& Translation : Meaning.Translations)
        {
            CollectTranslation(Translation);
        }
    }

    Entry.TranslationRu = FString::Join(RussianTranslations, TEXT(", "));
    Entry.TranslationUa = FString::Join(UkrainianTranslations, TEXT(", "));
    Entry.bHasUsageExamples = EVVocabularyUsage::HasUsageExamples(Entry.Usage);
    return Entry;
}
} // namespace EVVocabularyLegacyCompatibility
