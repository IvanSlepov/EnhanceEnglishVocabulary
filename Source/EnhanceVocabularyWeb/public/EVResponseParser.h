#pragma once

#include "CoreMinimal.h"
#include "EVVocabularyTypes.h"

class ENHANCEVOCABULARYWEB_API FEVResponseParser
{
public:
    static bool ParseFreeDictionaryResponse(const FString& JsonString, FWordSearchResult& OutResult);

    static bool ParseMyMemoryTranslationResponse(const FString& JsonString, FString& OutTranslation);

private:
    static bool IsValidTranslationCandidate(const FString& Translation);
};