#pragma once

#include "CoreMinimal.h"
#include "EVVocabularyTypes.h"
#include "Algo/Unique.h"
#include "EVVocabularyFilterTypes.generated.h"

UENUM(BlueprintType)
enum class EEVVocabularyFilterCategory : uint8
{
    None,
    PartOfSpeech
};

UENUM(BlueprintType)
enum class EEVVocabularyFilterOption : uint8
{
    None,
    PartOfSpeech_Noun,
    PartOfSpeech_Verb,
    PartOfSpeech_Adjective,
    PartOfSpeech_Adverb,
    PartOfSpeech_Pronoun,
    PartOfSpeech_Preposition,
    PartOfSpeech_Conjunction,
    PartOfSpeech_Interjection,
    PartOfSpeech_Determiner,
    PartOfSpeech_Numeral,
    PartOfSpeech_Auxiliary,
    PartOfSpeech_Particle,
    PartOfSpeech_PhrasalVerb,
    PartOfSpeech_ProperNoun,
    PartOfSpeech_Unspecified
};

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYCORE_API FEVVocabularyFilterCategorySelection
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEVVocabularyFilterCategory Category = EEVVocabularyFilterCategory::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EEVVocabularyFilterOption> SelectedOptions;

    bool IsValid() const
    {
        return Category != EEVVocabularyFilterCategory::None &&
               SelectedOptions.ContainsByPredicate([](const EEVVocabularyFilterOption Option)
                                                   { return Option != EEVVocabularyFilterOption::None; });
    }
};

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYCORE_API FEVVocabularyQueryCriteria
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FEVVocabularyFilterCategorySelection> Categories;

    bool HasActiveFilters() const
    {
        return Categories.ContainsByPredicate([](const FEVVocabularyFilterCategorySelection& Selection)
                                              { return Selection.IsValid(); });
    }

    void Normalize()
    {
        for (FEVVocabularyFilterCategorySelection& Selection : Categories)
        {
            Selection.SelectedOptions.RemoveAll([](const EEVVocabularyFilterOption Option)
                                                { return Option == EEVVocabularyFilterOption::None; });
            Selection.SelectedOptions.Sort([](const EEVVocabularyFilterOption A, const EEVVocabularyFilterOption B)
                                           { return static_cast<uint8>(A) < static_cast<uint8>(B); });
            Selection.SelectedOptions.SetNum(Algo::Unique(Selection.SelectedOptions));
        }

        Categories.RemoveAll([](const FEVVocabularyFilterCategorySelection& Selection)
                             { return !Selection.IsValid(); });
        Categories.Sort([](const FEVVocabularyFilterCategorySelection& A, const FEVVocabularyFilterCategorySelection& B)
                        { return static_cast<uint8>(A.Category) < static_cast<uint8>(B.Category); });
    }

    const FEVVocabularyFilterCategorySelection* FindCategory(const EEVVocabularyFilterCategory Category) const
    {
        return Categories.FindByPredicate([Category](const FEVVocabularyFilterCategorySelection& Selection)
                                          { return Selection.Category == Category; });
    }
};

namespace EVVocabularyFilter
{
inline FText GetCategoryDisplayText(const EEVVocabularyFilterCategory Category)
{
    switch (Category)
    {
    case EEVVocabularyFilterCategory::PartOfSpeech:
        return NSLOCTEXT("EVVocabularyFilter", "PartOfSpeechCategory", "Part of speech");
    default:
        return NSLOCTEXT("EVVocabularyFilter", "UnknownCategory", "Unknown category");
    }
}

inline FText GetOptionDisplayText(const EEVVocabularyFilterOption Option)
{
    switch (Option)
    {
    case EEVVocabularyFilterOption::PartOfSpeech_Noun:
        return NSLOCTEXT("EVVocabularyFilter", "Noun", "Noun");
    case EEVVocabularyFilterOption::PartOfSpeech_Verb:
        return NSLOCTEXT("EVVocabularyFilter", "Verb", "Verb");
    case EEVVocabularyFilterOption::PartOfSpeech_Adjective:
        return NSLOCTEXT("EVVocabularyFilter", "Adjective", "Adjective");
    case EEVVocabularyFilterOption::PartOfSpeech_Adverb:
        return NSLOCTEXT("EVVocabularyFilter", "Adverb", "Adverb");
    case EEVVocabularyFilterOption::PartOfSpeech_Pronoun:
        return NSLOCTEXT("EVVocabularyFilter", "Pronoun", "Pronoun");
    case EEVVocabularyFilterOption::PartOfSpeech_Preposition:
        return NSLOCTEXT("EVVocabularyFilter", "Preposition", "Preposition");
    case EEVVocabularyFilterOption::PartOfSpeech_Conjunction:
        return NSLOCTEXT("EVVocabularyFilter", "Conjunction", "Conjunction");
    case EEVVocabularyFilterOption::PartOfSpeech_Interjection:
        return NSLOCTEXT("EVVocabularyFilter", "Interjection", "Interjection");
    case EEVVocabularyFilterOption::PartOfSpeech_Determiner:
        return NSLOCTEXT("EVVocabularyFilter", "Determiner", "Determiner");
    case EEVVocabularyFilterOption::PartOfSpeech_Numeral:
        return NSLOCTEXT("EVVocabularyFilter", "Numeral", "Numeral");
    case EEVVocabularyFilterOption::PartOfSpeech_Auxiliary:
        return NSLOCTEXT("EVVocabularyFilter", "Auxiliary", "Auxiliary");
    case EEVVocabularyFilterOption::PartOfSpeech_Particle:
        return NSLOCTEXT("EVVocabularyFilter", "Particle", "Particle");
    case EEVVocabularyFilterOption::PartOfSpeech_PhrasalVerb:
        return NSLOCTEXT("EVVocabularyFilter", "PhrasalVerb", "Phrasal verb");
    case EEVVocabularyFilterOption::PartOfSpeech_ProperNoun:
        return NSLOCTEXT("EVVocabularyFilter", "ProperNoun", "Proper noun");
    case EEVVocabularyFilterOption::PartOfSpeech_Unspecified:
        return NSLOCTEXT("EVVocabularyFilter", "Unspecified", "Unspecified");
    default:
        return NSLOCTEXT("EVVocabularyFilter", "UnknownOption", "Unknown option");
    }
}

inline FString GetOptionDatabaseValue(const EEVVocabularyFilterOption Option)
{
    switch (Option)
    {
    case EEVVocabularyFilterOption::PartOfSpeech_Noun:
        return TEXT("noun");
    case EEVVocabularyFilterOption::PartOfSpeech_Verb:
        return TEXT("verb");
    case EEVVocabularyFilterOption::PartOfSpeech_Adjective:
        return TEXT("adjective");
    case EEVVocabularyFilterOption::PartOfSpeech_Adverb:
        return TEXT("adverb");
    case EEVVocabularyFilterOption::PartOfSpeech_Pronoun:
        return TEXT("pronoun");
    case EEVVocabularyFilterOption::PartOfSpeech_Preposition:
        return TEXT("preposition");
    case EEVVocabularyFilterOption::PartOfSpeech_Conjunction:
        return TEXT("conjunction");
    case EEVVocabularyFilterOption::PartOfSpeech_Interjection:
        return TEXT("interjection");
    case EEVVocabularyFilterOption::PartOfSpeech_Determiner:
        return TEXT("determiner");
    case EEVVocabularyFilterOption::PartOfSpeech_Numeral:
        return TEXT("numeral");
    case EEVVocabularyFilterOption::PartOfSpeech_Auxiliary:
        return TEXT("auxiliary");
    case EEVVocabularyFilterOption::PartOfSpeech_Particle:
        return TEXT("particle");
    case EEVVocabularyFilterOption::PartOfSpeech_PhrasalVerb:
        return TEXT("phrasal verb");
    case EEVVocabularyFilterOption::PartOfSpeech_ProperNoun:
        return TEXT("proper noun");
    case EEVVocabularyFilterOption::PartOfSpeech_Unspecified:
        return TEXT("unspecified");
    default:
        return FString();
    }
}

inline TArray<EEVVocabularyFilterOption> GetOptionsForCategory(const EEVVocabularyFilterCategory Category)
{
    if (Category != EEVVocabularyFilterCategory::PartOfSpeech)
    {
        return {};
    }

    return {EEVVocabularyFilterOption::PartOfSpeech_Noun,        EEVVocabularyFilterOption::PartOfSpeech_Verb,
            EEVVocabularyFilterOption::PartOfSpeech_Adjective,   EEVVocabularyFilterOption::PartOfSpeech_Adverb,
            EEVVocabularyFilterOption::PartOfSpeech_Pronoun,     EEVVocabularyFilterOption::PartOfSpeech_Preposition,
            EEVVocabularyFilterOption::PartOfSpeech_Conjunction, EEVVocabularyFilterOption::PartOfSpeech_Interjection,
            EEVVocabularyFilterOption::PartOfSpeech_Determiner,  EEVVocabularyFilterOption::PartOfSpeech_Numeral,
            EEVVocabularyFilterOption::PartOfSpeech_Auxiliary,   EEVVocabularyFilterOption::PartOfSpeech_Particle,
            EEVVocabularyFilterOption::PartOfSpeech_PhrasalVerb, EEVVocabularyFilterOption::PartOfSpeech_ProperNoun,
            EEVVocabularyFilterOption::PartOfSpeech_Unspecified};
}

inline TArray<FString> GetPartOfSpeechDatabaseValues(const FEVVocabularyQueryCriteria& Criteria)
{
    TArray<FString> Values;
    if (const FEVVocabularyFilterCategorySelection* Selection =
            Criteria.FindCategory(EEVVocabularyFilterCategory::PartOfSpeech))
    {
        for (const EEVVocabularyFilterOption Option : Selection->SelectedOptions)
        {
            const FString Value = GetOptionDatabaseValue(Option);
            if (!Value.IsEmpty())
            {
                Values.AddUnique(Value);
            }
        }
    }
    return Values;
}

inline bool MeaningMatchesCriteria(const FEVVocabularyMeaning& Meaning, const FEVVocabularyQueryCriteria& Criteria)
{
    const TArray<FString> AllowedPartsOfSpeech = GetPartOfSpeechDatabaseValues(Criteria);
    if (AllowedPartsOfSpeech.IsEmpty())
    {
        return true;
    }

    const FString NormalizedPartOfSpeech = Meaning.PartOfSpeech.TrimStartAndEnd().ToLower();
    return AllowedPartsOfSpeech.ContainsByPredicate(
        [&NormalizedPartOfSpeech](const FString& Value)
        { return NormalizedPartOfSpeech.Equals(Value, ESearchCase::IgnoreCase); });
}
} // namespace EVVocabularyFilter
