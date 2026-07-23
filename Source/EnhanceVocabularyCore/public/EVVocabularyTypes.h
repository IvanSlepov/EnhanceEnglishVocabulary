#pragma once

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

USTRUCT(BlueprintType)
struct FVocabularyEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Word;

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

USTRUCT(BlueprintType)
struct FWordSearchResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Word;

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
};