#include "EVVocabularyDatabaseSchema.h"

const FString& FEVVocabularyDatabaseSchema::GetTableName()
{
    static const FString TableName = TEXT("VocabularyEntries");

    return TableName;
}

const TArray<FEVDatabaseColumnDefinition>& FEVVocabularyDatabaseSchema::GetColumns()
{
    static const TArray<FEVDatabaseColumnDefinition> Columns = {
        {TEXT("Id"), TEXT("INTEGER PRIMARY KEY AUTOINCREMENT"), EEVDatabaseColumnRole::Internal},
        {TEXT("Word"), TEXT("TEXT NOT NULL UNIQUE"), EEVDatabaseColumnRole::ImportExport},
        {TEXT("Definition"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport},
        {TEXT("Usage"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport},
        {TEXT("TranslationRu"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport},
        {TEXT("TranslationUa"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport},
        {TEXT("CreatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"), EEVDatabaseColumnRole::Internal},
        {TEXT("UpdatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"), EEVDatabaseColumnRole::Internal}};

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