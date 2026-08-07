#pragma once

#include "CoreMinimal.h"
#include "EVVocabularyTypes.h"

class ENHANCEVOCABULARYWEB_API FEVResponseParser
{
public:
    static bool ParseFreeDictionaryResponse(const FString& JsonString, FEVVocabularyRecord& OutRecord);

    static bool ParseMyMemoryTranslationResponse(const FString& JsonString, const FString& TargetLanguage,
                                                 TArray<FEVVocabularyTranslation>& OutTranslations);

private:
    static bool IsValidTranslationCandidate(const FString& Translation);
};
