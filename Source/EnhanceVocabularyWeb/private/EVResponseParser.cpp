#include "EVResponseParser.h"

#include "EVJsonUtils.h"
#include "EVResponseTypes.h"
#include "Dom/JsonObject.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

bool FEVResponseParser::ParseFreeDictionaryResponse(const FString& JsonString, FWordSearchResult& OutResult)
{
    TArray<TSharedPtr<FJsonValue>> JsonArray;

    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, JsonArray) || JsonArray.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse FreeDictionary JSON array."));
        return false;
    }

    FEVFreeDictionaryResponse Response;

    if (!JsonArray[0].IsValid() || !JsonArray[0]->AsObject().IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("FreeDictionary first array item is invalid."));
        return false;
    }

    if (!FJsonObjectConverter::JsonObjectToUStruct(JsonArray[0]->AsObject().ToSharedRef(), &Response))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to convert FreeDictionary object to struct."));
        return false;
    }

    OutResult.Word = Response.Word;

    FString DefinitionText;
    FString UsageText;

    int32 Index = 1;

    for (const FEVFreeDictionaryMeaningGroup& Meaning : Response.Meanings)
    {
        for (const FEVFreeDictionaryDefinitionItem& Definition : Meaning.Definitions)
        {
            DefinitionText += FString::Printf(TEXT("%d. %s\n"), Index, *Definition.Definition);

            if (!Definition.Example.IsEmpty())
            {
                UsageText += FString::Printf(TEXT("%d. %s\n"), Index, *Definition.Example);
            }
            else
            {
                UsageText += FString::Printf(TEXT("%d. No usage example provided.\n"), Index);
            }

            ++Index;
        }
    }

    OutResult.Definition = DefinitionText.TrimEnd();
    OutResult.Usage = UsageText.TrimEnd();

    return true;
}

bool FEVResponseParser::ParseMyMemoryTranslationResponse(const FString& JsonString, FString& OutTranslation)
{
    FEVMyMemoryResponse Response;

    if (!FEVJsonUtils::JsonStringToStruct(JsonString, Response))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse MyMemory response."));
        return false;
    }

    if (Response.ResponseStatus != 200)
    {
        UE_LOG(LogTemp, Error, TEXT("MyMemory response status is not 200: %d"), Response.ResponseStatus);
        return false;
    }

    if (IsValidTranslationCandidate(Response.ResponseData.TranslatedText))
    {
        OutTranslation = Response.ResponseData.TranslatedText;
        return true;
    }

    for (const FEVMyMemoryMatchItem& Match : Response.Matches)
    {
        if (IsValidTranslationCandidate(Match.Translation))
        {
            OutTranslation = Match.Translation;
            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("No valid MyMemory translation found."));
    return false;
}

bool FEVResponseParser::IsValidTranslationCandidate(const FString& Translation)
{
    const FString Clean = Translation.TrimStartAndEnd();

    if (Clean.IsEmpty())
    {
        return false;
    }

    if (Clean == TEXT("?"))
    {
        return false;
    }

    if (Clean.Contains(TEXT("<g id=")))
    {
        return false;
    }

    return true;
}