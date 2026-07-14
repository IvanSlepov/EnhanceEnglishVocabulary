#pragma once

#include "CoreMinimal.h"
#include "EVFileExchangeTypes.h"

class ENHANCEVOCABULARYCORE_API FEVFileExchangeDefaults
{
public:
    static FString GetTemplateFileName(EEVFileExtensionType FileExtensionType);

    static FString GetDatabaseExportFileName(EEVFileExtensionType FileExtensionType);

    static FString GetValidationReportFileName(EEVFileExtensionType FileExtensionType);
};