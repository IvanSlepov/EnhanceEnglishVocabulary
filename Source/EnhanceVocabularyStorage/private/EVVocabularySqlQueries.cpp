#include "EVVocabularySqlQueries.h"

#include "EVVocabularyDatabaseSchema.h"
#include "EVVocabularyFieldRegistry.h"

namespace
{
FString BuildColumnList()
{
    return FString::Join(FEVVocabularyFieldRegistry::GetPersistentColumnNames(), TEXT(", "));
}

FString BuildPlaceholders(int32 Count)
{
    TArray<FString> Placeholders;
    Placeholders.Init(TEXT("?"), Count);
    return FString::Join(Placeholders, TEXT(", "));
}
} // namespace

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

FString FEVVocabularySqlQueries::GetInsertVocabularyEntryStrictQuery()
{
    const int32 FieldCount = FEVVocabularyFieldRegistry::GetPersistentFields().Num();

    return FString::Printf(TEXT("INSERT INTO %s (%s) VALUES (%s);"), *FEVVocabularyDatabaseSchema::GetTableName(),
                           *BuildColumnList(), *BuildPlaceholders(FieldCount));
}

FString FEVVocabularySqlQueries::GetEditVocabularyEntryQuery()
{
    TArray<FString> Assignments;

    for (const FEVDatabaseColumnDefinition& Field : FEVVocabularyFieldRegistry::GetEditablePersistentFields())
    {
        Assignments.Add(Field.Name + TEXT(" = ?"));
    }

    return FString::Printf(TEXT("UPDATE %s SET %s WHERE Word = ?;"), *FEVVocabularyDatabaseSchema::GetTableName(),
                           *FString::Join(Assignments, TEXT(", ")));
}

FString FEVVocabularySqlQueries::GetVocabularyEntryByWordQuery()
{
    return FString::Printf(TEXT("SELECT %s FROM %s WHERE Word = ?;"), *BuildColumnList(),
                           *FEVVocabularyDatabaseSchema::GetTableName());
}

FString FEVVocabularySqlQueries::GetSelectImportExportColumnsQuery()
{
    const FString ColumnList = BuildColumnList();

    if (ColumnList.IsEmpty())
    {
        return FString();
    }

    return FString::Printf(TEXT("SELECT %s FROM %s;"), *ColumnList, *FEVVocabularyDatabaseSchema::GetTableName());
}

FString FEVVocabularySqlQueries::GetSelectVocabularyEntriesQuery()
{
    return FString::Printf(TEXT("SELECT %s FROM %s ORDER BY Word ASC;"), *BuildColumnList(),
                           *FEVVocabularyDatabaseSchema::GetTableName());
}

FString FEVVocabularySqlQueries::GetSelectVocabularyEntriesPageQuery()
{
    return FString::Printf(TEXT("SELECT %s FROM %s ORDER BY Word COLLATE NOCASE ASC LIMIT ? OFFSET ?;"),
                           *BuildColumnList(), *FEVVocabularyDatabaseSchema::GetTableName());
}

FString FEVVocabularySqlQueries::GetSelectVocabularyEntriesPageByPrefixQuery()
{
    return FString::Printf(
        TEXT("SELECT %s FROM %s WHERE Word LIKE ? COLLATE NOCASE ORDER BY Word COLLATE NOCASE ASC LIMIT ? OFFSET ?;"),
        *BuildColumnList(), *FEVVocabularyDatabaseSchema::GetTableName());
}
