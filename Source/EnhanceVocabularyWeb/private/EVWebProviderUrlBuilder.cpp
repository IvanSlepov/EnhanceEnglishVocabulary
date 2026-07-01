#include "EVWebProviderUrlBuilder.h"

#include "EVWebProviderRegistry.h"

bool FEVWebProviderUrlBuilder::BuildRequestUrl(EEVWebProvider Provider, const FEVWebProviderUrlBuildContext& Context,
                                               FString& OutUrl)
{
    const FEVWebProviderInfo ProviderInfo = FEVWebProviderRegistry::GetProviderInfo(Provider);

    if (!ProviderInfo.bEnabled)
    {
        UE_LOG(LogTemp, Error, TEXT("Selected web provider is disabled"));
        return false;
    }

    switch (Provider)
    {
    case EEVWebProvider::FreeDictionary:
        if (Context.Word.IsEmpty())
        {
            UE_LOG(LogTemp, Error, TEXT("FreeDictionary URL build failed: Word is empty"));
            return false;
        }

        OutUrl = ProviderInfo.BaseUrl + Context.Word;
        return true;

    case EEVWebProvider::MyMemory:
        if (Context.Word.IsEmpty() || Context.SourceLanguage.IsEmpty() || Context.TargetLanguage.IsEmpty())
        {
            UE_LOG(LogTemp, Error, TEXT("MyMemory URL build failed: missing word or language data"));
            return false;
        }

        OutUrl = ProviderInfo.BaseUrl + TEXT("?q=") + Context.Word + TEXT("&langpair=") + Context.SourceLanguage +
                 TEXT("|") + Context.TargetLanguage;

        return true;

    default:
        UE_LOG(LogTemp, Error, TEXT("Unsupported provider for URL building"));
        return false;
    }
}