#pragma once

#include "EVVocabularyTypes.generated.h"

USTRUCT(BlueprintType)
struct FVocabularyEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Word;

    UPROPERTY(BlueprintReadWrite)
    FString Definition;

    UPROPERTY(BlueprintReadWrite)
    FString Usage;

    UPROPERTY(BlueprintReadWrite)
    FString TranslationRu;

    UPROPERTY(BlueprintReadWrite)
    FString TranslationUa;
};

USTRUCT(BlueprintType)
struct FWordSearchResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Word;

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
    FString ErrorMessage;
};