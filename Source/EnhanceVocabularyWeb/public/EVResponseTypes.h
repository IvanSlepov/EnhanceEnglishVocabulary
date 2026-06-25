#pragma once

#include "CoreMinimal.h"
#include "EVResponseTypes.generated.h"

/*
 * FreeDictionary API
 * Response is an array:
 * [
 *   {
 *     "word": "...",
 *     "meanings": [
 *       {
 *         "partOfSpeech": "...",
 *         "definitions": [
 *           {
 *             "definition": "...",
 *             "example": "..."
 *           }
 *         ]
 *       }
 *     ]
 *   }
 * ]
 */

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYWEB_API FEVFreeDictionaryDefinitionItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Definition;

    // Maps FreeDictionary "example" field.
    // In our app this becomes FWordSearchResult::Usage.
    UPROPERTY(BlueprintReadWrite)
    FString Example;
};

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYWEB_API FEVFreeDictionaryMeaningGroup
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString PartOfSpeech;

    UPROPERTY(BlueprintReadWrite)
    TArray<FEVFreeDictionaryDefinitionItem> Definitions;
};

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYWEB_API FEVFreeDictionaryResponse
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Word;

    UPROPERTY(BlueprintReadWrite)
    TArray<FEVFreeDictionaryMeaningGroup> Meanings;
};

/*
 * MyMemory Translation API
 * Same response structure for:
 * en|ru
 * en|uk
 */

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYWEB_API FEVMyMemoryResponseData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString TranslatedText;

    UPROPERTY(BlueprintReadWrite)
    float Match = 0.0f;
};

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYWEB_API FEVMyMemoryMatchItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Segment;

    UPROPERTY(BlueprintReadWrite)
    FString Translation;

    UPROPERTY(BlueprintReadWrite)
    FString Source;

    UPROPERTY(BlueprintReadWrite)
    FString Target;

    // MyMemory sometimes returns quality as string, sometimes as number.
    // Keep as FString for safer parsing.
    UPROPERTY(BlueprintReadWrite)
    FString Quality;

    UPROPERTY(BlueprintReadWrite)
    float Match = 0.0f;
};

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYWEB_API FEVMyMemoryResponse
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FEVMyMemoryResponseData ResponseData;

    UPROPERTY(BlueprintReadWrite)
    bool QuotaFinished = false;

    UPROPERTY(BlueprintReadWrite)
    FString ResponseDetails;

    UPROPERTY(BlueprintReadWrite)
    int32 ResponseStatus = 0;

    UPROPERTY(BlueprintReadWrite)
    TArray<FEVMyMemoryMatchItem> Matches;
};