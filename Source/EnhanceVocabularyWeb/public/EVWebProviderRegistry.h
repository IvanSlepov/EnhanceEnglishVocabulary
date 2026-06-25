#pragma once

#include "CoreMinimal.h"
#include "EVWebProviderTypes.h"

class ENHANCEVOCABULARYWEB_API FEVWebProviderRegistry
{

public:
    static FEVWebProviderInfo GetProviderInfo(EEVWebProvider Provider);
};