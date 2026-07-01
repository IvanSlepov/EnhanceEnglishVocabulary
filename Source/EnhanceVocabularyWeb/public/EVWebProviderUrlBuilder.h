#pragma once

#include "CoreMinimal.h"
#include "EVWebProviderTypes.h"

struct ENHANCEVOCABULARYWEB_API FEVWebProviderUrlBuildContext
{
    FString Word;
    FString SourceLanguage = TEXT("en");
    FString TargetLanguage;
};

class ENHANCEVOCABULARYWEB_API FEVWebProviderUrlBuilder
{
public:
    static bool BuildRequestUrl(EEVWebProvider Provider, const FEVWebProviderUrlBuildContext& Context, FString& OutUrl);
};