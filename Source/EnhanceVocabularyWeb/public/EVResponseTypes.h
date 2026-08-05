#pragma once

#include "CoreMinimal.h"
#include "EVResponseTypes.generated.h"

/* FreeDictionary provider DTOs. */

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYWEB_API FEVFreeDictionaryLicense
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    FString Url;
};

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYWEB_API FEVFreeDictionaryDefinitionItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Definition;

    UPROPERTY(BlueprintReadWrite)
    FString Example;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> Synonyms;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> Antonyms;
};

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYWEB_API FEVFreeDictionaryMeaningGroup
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString PartOfSpeech;

    UPROPERTY(BlueprintReadWrite)
    TArray<FEVFreeDictionaryDefinitionItem> Definitions;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> Synonyms;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> Antonyms;
};

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYWEB_API FEVFreeDictionaryPhonetic
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Text;

    UPROPERTY(BlueprintReadWrite)
    FString Audio;

    UPROPERTY(BlueprintReadWrite)
    FString SourceUrl;

    UPROPERTY(BlueprintReadWrite)
    FEVFreeDictionaryLicense License;
};

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYWEB_API FEVFreeDictionaryResponse
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Word;

    UPROPERTY(BlueprintReadWrite)
    FString Phonetic;

    UPROPERTY(BlueprintReadWrite)
    TArray<FEVFreeDictionaryPhonetic> Phonetics;

    UPROPERTY(BlueprintReadWrite)
    TArray<FEVFreeDictionaryMeaningGroup> Meanings;

    UPROPERTY(BlueprintReadWrite)
    FEVFreeDictionaryLicense License;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> SourceUrls;
};

/* MyMemory translation provider DTOs. */

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
