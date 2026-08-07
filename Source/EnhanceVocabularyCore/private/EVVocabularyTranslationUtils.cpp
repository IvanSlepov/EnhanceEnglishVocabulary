#include "EVVocabularyTranslationUtils.h"

namespace EVVocabularyTranslationUtils
{
void SplitTranslationValues(const FString& Source, TArray<FString>& OutValues)
{
    OutValues.Reset();

    FString Normalized = Source;
    Normalized.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
    Normalized.ReplaceInline(TEXT("\r"), TEXT("\n"));
    Normalized.ReplaceInline(TEXT(";"), TEXT("\n"));
    Normalized.ReplaceInline(TEXT(","), TEXT("\n"));

    TArray<FString> RawValues;
    Normalized.ParseIntoArray(RawValues, TEXT("\n"), true);

    for (FString Value : RawValues)
    {
        Value = Value.TrimStartAndEnd();
        if (Value.IsEmpty())
        {
            continue;
        }

        const bool bDuplicate = OutValues.ContainsByPredicate(
            [&Value](const FString& Existing) { return Existing.Equals(Value, ESearchCase::IgnoreCase); });

        if (!bDuplicate)
        {
            OutValues.Add(MoveTemp(Value));
        }
    }
}

void ExpandTranslation(const FEVVocabularyTranslation& Source, TArray<FEVVocabularyTranslation>& OutTranslations)
{
    TArray<FString> Values;
    SplitTranslationValues(Source.TranslationText, Values);

    for (const FString& Value : Values)
    {
        FEVVocabularyTranslation Expanded = Source;
        Expanded.TranslationText = Value;
        OutTranslations.Add(MoveTemp(Expanded));
    }
}

void NormalizeTranslations(TArray<FEVVocabularyTranslation>& Translations)
{
    TArray<FEVVocabularyTranslation> Normalized;

    for (const FEVVocabularyTranslation& Source : Translations)
    {
        TArray<FEVVocabularyTranslation> Expanded;
        ExpandTranslation(Source, Expanded);

        for (FEVVocabularyTranslation& Candidate : Expanded)
        {
            const bool bDuplicate = Normalized.ContainsByPredicate(
                [&Candidate](const FEVVocabularyTranslation& Existing)
                {
                    return Existing.TargetLanguage.Equals(Candidate.TargetLanguage, ESearchCase::IgnoreCase) &&
                           Existing.TargetPartOfSpeech.Equals(Candidate.TargetPartOfSpeech, ESearchCase::IgnoreCase) &&
                           Existing.TranslationText.Equals(Candidate.TranslationText, ESearchCase::IgnoreCase);
                });

            if (bDuplicate)
            {
                continue;
            }

            Candidate.DisplayOrder = Normalized.Num();
            Normalized.Add(MoveTemp(Candidate));
        }
    }

    Translations = MoveTemp(Normalized);
}
} // namespace EVVocabularyTranslationUtils
