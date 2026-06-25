#include "EVWebProviderRegistry.h"
#include "EVWebProviders.h"

FEVWebProviderInfo FEVWebProviderRegistry::GetProviderInfo(EEVWebProvider Provider)
{
    FEVWebProviderInfo Info;

    switch (Provider)
    {
    case EEVWebProvider::FreeDictionary:
        Info.Provider = Provider;
        Info.BaseUrl = EVWebProviders::FreeDictionary;
        Info.ProviderType = EEVWebProviderType::Dictionary;
        Info.RequestVerb = EEVWebRequestVerb::GET;
        break;

    case EEVWebProvider::MyMemory:
        Info.Provider = Provider;
        Info.BaseUrl = EVWebProviders::MyMemory;
        Info.ProviderType = EEVWebProviderType::Translation;
        Info.RequestVerb = EEVWebRequestVerb::GET;
        break;

    case EEVWebProvider::Datamuse:
        Info.Provider = Provider;
        Info.BaseUrl = EVWebProviders::Datamuse;
        Info.ProviderType = EEVWebProviderType::Metadata;
        Info.RequestVerb = EEVWebRequestVerb::GET;
        break;

    default:
        break;
    }

    return Info;
}