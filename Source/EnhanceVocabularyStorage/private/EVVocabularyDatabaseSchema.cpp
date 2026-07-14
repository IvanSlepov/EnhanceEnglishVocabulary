#include "EVVocabularyDatabaseSchema.h"

const FString& FEVVocabularyDatabaseSchema::GetTableName()
{
    static const FString TableName = TEXT("VocabularyEntries");

    return TableName;
}

const TArray<FEVDatabaseColumnDefinition>& FEVVocabularyDatabaseSchema::GetColumns()
{
    // We're adding the fourth, TEXT, parameter to notify user of the rules for each column.
    // This info will be added both to the DownloadDB template and the auto-generated validation report(for the imported
    // files)
    static const TArray<FEVDatabaseColumnDefinition> Columns = {
        {TEXT("Id"), TEXT("INTEGER PRIMARY KEY AUTOINCREMENT"), EEVDatabaseColumnRole::Internal, TEXT("")},
        {TEXT("Word"), TEXT("TEXT NOT NULL UNIQUE"), EEVDatabaseColumnRole::ImportExport,
         TEXT("WORD - can't be empty!!!")},
        {TEXT("Definition"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, TEXT("Definition - can be empty")},
        {TEXT("Usage"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, TEXT("Usage - can be empty")},
        {TEXT("TranslationRu"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport,
         TEXT("TranslationRU - can be empty")},
        {TEXT("TranslationUa"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport,
         TEXT("TranslationUA - can be empty")},
        {TEXT("CreatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"), EEVDatabaseColumnRole::Internal, TEXT("")},
        {TEXT("UpdatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"), EEVDatabaseColumnRole::Internal,
         TEXT("")}};

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