#include "EVVocabularyDatabaseSchema.h"

const FString& FEVVocabularyDatabaseSchema::GetTableName()
{
    static const FString TableName = TEXT("VocabularyEntries");
    return TableName;
}

const TArray<FEVDatabaseColumnDefinition>& FEVVocabularyDatabaseSchema::GetColumns()
{
    static const TArray<FEVDatabaseColumnDefinition> Columns = []
    {
        TArray<FEVDatabaseColumnDefinition> Result;

        Result.Add({TEXT("Id"), TEXT("INTEGER PRIMARY KEY AUTOINCREMENT"), EEVDatabaseColumnRole::Internal,
                    EEVImportRequirement::None, TEXT(""), nullptr, false});

        Result.Append(FEVVocabularyFieldRegistry::GetPersistentFields());

        Result.Add({TEXT("CreatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"), EEVDatabaseColumnRole::Internal,
                    EEVImportRequirement::None, TEXT(""), nullptr, false});
        Result.Add({TEXT("UpdatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"), EEVDatabaseColumnRole::Internal,
                    EEVImportRequirement::None, TEXT(""), nullptr, false});

        return Result;
    }();

    return Columns;
}

TArray<FEVDatabaseColumnDefinition> FEVVocabularyDatabaseSchema::GetImportExportColumns()
{
    return FEVVocabularyFieldRegistry::GetPersistentFields();
}

TArray<FString> FEVVocabularyDatabaseSchema::GetImportExportColumnNames()
{
    return FEVVocabularyFieldRegistry::GetPersistentColumnNames();
}
