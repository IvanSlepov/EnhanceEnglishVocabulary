#pragma once

#include "CoreMinimal.h"
#include "EVVocabularyLanguageTypes.generated.h"

/**
 * Database-backed vocabulary contexts.
 *
 * Only English USA is enabled in the current release. Additional contexts can
 * be appended later without changing the translation-language contract.
 */
UENUM(BlueprintType)
enum class EEVVocabularyDBContext : uint8
{
    None UMETA(DisplayName = "None"),
    EnglishUSA UMETA(DisplayName = "English USA")
};

/**
 * Translation flavours supported by the application UI and web pipeline.
 * A translation flavour is intentionally distinct from a database context.
 */
UENUM(BlueprintType)
enum class EEVVocabularyTranslationLanguage : uint8
{
    None UMETA(DisplayName = "None"),
    EnglishUSA UMETA(DisplayName = "English USA"),
    EnglishUK UMETA(DisplayName = "English UK"),
    Russian UMETA(DisplayName = "Russian"),
    Ukrainian UMETA(DisplayName = "Ukrainian"),
    Spanish UMETA(DisplayName = "Spanish"),
    German UMETA(DisplayName = "German"),
    French UMETA(DisplayName = "French"),
    Italian UMETA(DisplayName = "Italian")
};

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYCORE_API FEVVocabularyLanguagePreferences
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEVVocabularyDBContext DatabaseContext = EEVVocabularyDBContext::EnglishUSA;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EEVVocabularyTranslationLanguage> SelectedTranslations;

    FEVVocabularyLanguagePreferences()
    {
        SelectedTranslations = {EEVVocabularyTranslationLanguage::Russian, EEVVocabularyTranslationLanguage::Ukrainian};
    }

    void Normalize();

    bool HasSelectedTranslations() const
    {
        return !SelectedTranslations.IsEmpty();
    }

    bool operator==(const FEVVocabularyLanguagePreferences& Other) const
    {
        return DatabaseContext == Other.DatabaseContext && SelectedTranslations == Other.SelectedTranslations;
    }

    bool operator!=(const FEVVocabularyLanguagePreferences& Other) const
    {
        return !(*this == Other);
    }
};

namespace EVVocabularyLanguage
{
ENHANCEVOCABULARYCORE_API const TArray<EEVVocabularyDBContext>& GetAvailableDatabaseContexts();
ENHANCEVOCABULARYCORE_API const TArray<EEVVocabularyTranslationLanguage>& GetAvailableTranslationLanguages();

ENHANCEVOCABULARYCORE_API FText GetDatabaseContextDisplayText(EEVVocabularyDBContext Context);
ENHANCEVOCABULARYCORE_API FText GetTranslationLanguageDisplayText(EEVVocabularyTranslationLanguage Language);

/** Stable persisted identifier. */
ENHANCEVOCABULARYCORE_API FString GetDatabaseContextId(EEVVocabularyDBContext Context);
/** Canonical value stored in FEVVocabularyTranslation::TargetLanguage. */
ENHANCEVOCABULARYCORE_API FString GetTranslationStorageCode(EEVVocabularyTranslationLanguage Language);
/** Provider-facing source language code. */
ENHANCEVOCABULARYCORE_API FString GetDatabaseContextWebLanguageCode(EEVVocabularyDBContext Context);
/** Provider-facing target language code. */
ENHANCEVOCABULARYCORE_API FString GetTranslationWebLanguageCode(EEVVocabularyTranslationLanguage Language);
/** Pronunciation rows currently use broad language codes such as "en". */
ENHANCEVOCABULARYCORE_API FString GetDatabaseContextPronunciationLanguageCode(EEVVocabularyDBContext Context);

ENHANCEVOCABULARYCORE_API bool TryParseDatabaseContext(const FString& Value, EEVVocabularyDBContext& OutContext);
ENHANCEVOCABULARYCORE_API bool TryParseTranslationLanguage(const FString& Value,
                                                           EEVVocabularyTranslationLanguage& OutLanguage);

ENHANCEVOCABULARYCORE_API bool IsTranslationEquivalentToDatabaseContext(EEVVocabularyTranslationLanguage Language,
                                                                        EEVVocabularyDBContext Context);

/** Returns true only when this translation flavour currently has a real DB context. */
ENHANCEVOCABULARYCORE_API bool TryResolveDatabaseContextForTranslation(EEVVocabularyTranslationLanguage Language,
                                                                       EEVVocabularyDBContext& OutContext);
} // namespace EVVocabularyLanguage
