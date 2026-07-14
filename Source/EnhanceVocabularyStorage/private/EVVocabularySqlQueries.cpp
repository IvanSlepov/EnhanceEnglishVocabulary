#include "EVVocabularySqlQueries.h"
#include "EVVocabularyDatabaseSchema.h"

FString FEVVocabularySqlQueries::GetCreateVocabularyTableQuery()
{
    const FString& TableName = FEVVocabularyDatabaseSchema::GetTableName();

    const TArray<FEVDatabaseColumnDefinition>& Columns = FEVVocabularyDatabaseSchema::GetColumns();

    FString Query = FString::Printf(TEXT("CREATE TABLE IF NOT EXISTS %s ("), *TableName);

    for (int32 Index = 0; Index < Columns.Num(); ++Index)
    {
        const FEVDatabaseColumnDefinition& Column = Columns[Index];

        Query += FString::Printf(TEXT("%s %s"), *Column.Name, *Column.SqlDefinition);

        if (Index < Columns.Num() - 1)
        {
            Query += TEXT(",");
        }
    }

    Query += TEXT(");");

    return Query;
}

FString FEVVocabularySqlQueries::GetSelectImportExportColumnsQuery()
{
    const TArray<FString> ColumnNames = FEVVocabularyDatabaseSchema::GetImportExportColumnNames();

    if (ColumnNames.IsEmpty())
    {
        return FString();
    }

    return FString::Printf(TEXT("SELECT %s FROM %s;"), *FString::Join(ColumnNames, TEXT(", ")),
                           *FEVVocabularyDatabaseSchema::GetTableName());
}