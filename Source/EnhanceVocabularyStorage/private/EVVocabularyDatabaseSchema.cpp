#include "EVVocabularyDatabaseSchema.h"

const FString& FEVVocabularyDatabaseSchema::GetTableName()
{
    static const FString TableName = TEXT("VocabularyEntries");

    return TableName;
}

const TArray<FEVDatabaseColumnDefinition>& FEVVocabularyDatabaseSchema::GetColumns()
{
    // Import-related metadata.
    // Used to generate both:
    // 1. the downloadable import template;
    // 2. the validation report and import rules.
    static const TArray<FEVDatabaseColumnDefinition> Columns = {
        {TEXT("Id"), TEXT("INTEGER PRIMARY KEY AUTOINCREMENT"), EEVDatabaseColumnRole::Internal,
         EEVImportRequirement::None, TEXT("")},
        {TEXT("Word"), TEXT("TEXT NOT NULL UNIQUE"), EEVDatabaseColumnRole::ImportExport,
         EEVImportRequirement::Required, TEXT("WORD - can't be empty!!!")},
        {TEXT("Definition"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("Definition - can be empty")},
        {TEXT("Usage"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("Usage - can be empty")},
        {TEXT("TranslationRu"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("TranslationRU - can be empty")},
        {TEXT("TranslationUa"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("TranslationUA - can be empty")},
        {TEXT("CreatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"), EEVDatabaseColumnRole::Internal,
         EEVImportRequirement::None, TEXT("")},
        {TEXT("UpdatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"), EEVDatabaseColumnRole::Internal,
         EEVImportRequirement::None, TEXT("")}};

    return Columns;
}

TArray<FEVDatabaseColumnDefinition> FEVVocabularyDatabaseSchema::GetImportExportColumns()
{
    TArray<FEVDatabaseColumnDefinition> ImportExportColumns;

    for (const FEVDatabaseColumnDefinition& Column : GetColumns())
    {
        if (Column.Role == EEVDatabaseColumnRole::ImportExport)
        {
            ImportExportColumns.Add(Column);
        }
    }

    return ImportExportColumns;
}

TArray<FString> FEVVocabularyDatabaseSchema::GetImportExportColumnNames()
{
    TArray<FString> ColumnNames;

    for (const FEVDatabaseColumnDefinition& Column : GetColumns())
    {
        if (Column.Role == EEVDatabaseColumnRole::ImportExport)
        {
            ColumnNames.Add(Column.Name);
        }
    }

    return ColumnNames;
}