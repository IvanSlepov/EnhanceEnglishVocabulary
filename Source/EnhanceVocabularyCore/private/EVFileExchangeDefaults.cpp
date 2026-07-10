#include "EVFileExchangeDefaults.h"

FString FEVFileExchangeDefaults::GetTemplateFileName(EEVFileExtensionType FileExtensionType)
{
    switch (FileExtensionType)
    {
    case EEVFileExtensionType::Csv:
        return TEXT("VocabularyDB_Template.csv");

    default:
        return TEXT("VocabularyDB_Template");
    }
}

FString FEVFileExchangeDefaults::GetDatabaseExportFileName(EEVFileExtensionType FileExtensionType)
{
    switch (FileExtensionType)
    {
    case EEVFileExtensionType::Csv:
        return TEXT("VocabularyDB_Export.csv");

    default:
        return TEXT("VocabularyDB_Export");
    }
}