#pragma once

#include "CoreMinimal.h"
#include "EVVocabularyTypes.generated.h"

namespace EVVocabularyUsage
{
inline const FString& GetNoUsageExamplesText()
{
    static const FString Text = TEXT("No usage examples provided.");
    return Text;
}

inline bool HasUsageExamples(const FString& Usage)
{
    return !Usage.IsEmpty() && !Usage.Equals(GetNoUsageExamplesText(), ESearchCase::CaseSensitive);
}
} // namespace EVVocabularyUsage

/**
 * One pronunciation variant belonging to a vocabulary entry.
 *
 * Transcription and audio stay together because providers can return several
 * accent-specific phonetic/audio pairs for the same word.
 */
USTRUCT(BlueprintType)
struct FEVVocabularyPronunciation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LanguageCode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Transcription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AudioUrl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Accent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SourceUrl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LicenseName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LicenseUrl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPrimary = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DisplayOrder = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ProviderName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ProviderKey;
};

/** One definition and its directly associated usage example. */
USTRUCT(BlueprintType)
struct FEVVocabularyDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DefinitionText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString UsageExample;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DisplayOrder = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ProviderName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ProviderKey;
};

/**
 * One translated value. Several rows can share the same target language and
 * meaning, which avoids storing comma-separated values in the database.
 */
USTRUCT(BlueprintType)
struct FEVVocabularyTranslation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TranslationText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetLanguage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetPartOfSpeech;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DisplayOrder = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ProviderName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ProviderKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Confidence = 0.0f;
};

/** A synonym, antonym, or future lexical relation. */
USTRUCT(BlueprintType)
struct FEVVocabularyRelation
{
    GENERATED_BODY()

    // Presentation form. It can differ from NormalizedRelatedWord later.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RelatedWord;

    // Canonical identity used for lookup and duplicate checks.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NormalizedRelatedWord;

    // Text is intentionally used instead of a closed enum so new provider
    // relation categories can be introduced without a database migration.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RelationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DisplayOrder = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ProviderName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ProviderKey;
};

/**
 * A semantic block rendered as one clearly separated UI section.
 *
 * PartOfSpeech is a property of the meaning; the meaning is the repeated UI
 * and storage entity. Multiple meanings may later share the same part of
 * speech, so callers must not treat PartOfSpeech as a unique identifier.
 */
USTRUCT(BlueprintType)
struct FEVVocabularyMeaning
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PartOfSpeech = TEXT("unspecified");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DisplayOrder = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ProviderName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ProviderKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FEVVocabularyDefinition> Definitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FEVVocabularyTranslation> Translations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FEVVocabularyRelation> Relations;
};

/**
 * Canonical vocabulary aggregate owned by the application.
 * Providers populate all or part of this structure; storage persists it into
 * the normalized table hierarchy.
 */
USTRUCT(BlueprintType)
struct FEVVocabularyRecord
{
    GENERATED_BODY()

    // Presentation form. For the first migration both fields may contain the
    // same normalized text, but code must not assume that they are identical.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Word;

    // Canonical identity used for lookup, uniqueness, merging and relations.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NormalizedWord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FEVVocabularyPronunciation> Pronunciations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FEVVocabularyMeaning> Meanings;

    // Translation providers often cannot identify a source meaning. Those
    // values remain entry-level instead of being assigned speculatively.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FEVVocabularyTranslation> GeneralTranslations;
};

/**
 * Legacy flattened UI/import-export model.
 *
 * Keep this structure during the staged migration. It will be produced from
 * and converted into FEVVocabularyRecord by compatibility adapters.
 */
USTRUCT(BlueprintType)
struct FVocabularyEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Word;

    // Not yet part of the version-1 flat SQLite binding. It is introduced now
    // so consumers can migrate to canonical lookup without another model edit.
    UPROPERTY(BlueprintReadWrite)
    FString NormalizedWord;

    UPROPERTY(BlueprintReadWrite)
    FString Transcription;

    UPROPERTY(BlueprintReadWrite)
    FString Definition;

    UPROPERTY(BlueprintReadWrite)
    FString Usage;

    UPROPERTY(BlueprintReadWrite)
    FString TranslationRu;

    UPROPERTY(BlueprintReadWrite)
    FString TranslationUa;

    // UI metadata only. It is not stored as a database column.
    UPROPERTY(BlueprintReadWrite)
    bool bHasUsageExamples = false;
};

/** Legacy flattened online-search result retained during provider migration. */
USTRUCT(BlueprintType)
struct FWordSearchResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Word;

    UPROPERTY(BlueprintReadWrite)
    FString NormalizedWord;

    UPROPERTY(BlueprintReadWrite)
    FString Transcription;

    UPROPERTY(BlueprintReadWrite)
    FString Definition;

    UPROPERTY(BlueprintReadWrite)
    FString Usage;

    UPROPERTY(BlueprintReadWrite)
    FString TranslationRu;

    UPROPERTY(BlueprintReadWrite)
    FString TranslationUa;

    UPROPERTY(BlueprintReadWrite)
    bool bSuccess = false;

    UPROPERTY(BlueprintReadWrite)
    bool bHasUsageExamples = false;

    UPROPERTY(BlueprintReadWrite)
    FString ErrorMessage;

    // Canonical structured result used by normalized storage and the future
    // shared vocabulary-entry UI. The legacy fields above remain populated
    // until the UI migration is completed.
    UPROPERTY(BlueprintReadWrite)
    FEVVocabularyRecord VocabularyRecord;
};
