#pragma once

#include "CoreMinimal.h"
#include "EVVocabularyFieldRegistry.h"

/** One column in a normalized vocabulary table. */
struct FEVVocabularySchemaColumn
{
    FString Name;
    FString SqlDefinition;
};

/**
 * Complete table descriptor used by versioned schema creation and migration.
 * Table-level constraints are kept separately from columns so foreign keys and
 * uniqueness rules remain explicit and readable.
 */
struct FEVVocabularySchemaTable
{
    FString Name;
    TArray<FEVVocabularySchemaColumn> Columns;
    TArray<FString> Constraints;
    TArray<FString> IndexQueries;
};

class ENHANCEVOCABULARYSTORAGE_API FEVVocabularyDatabaseSchema
{
public:
    /** Current active flat-table API. Retained until the v1 -> v2 migration lands. */
    static const FString& GetTableName();
    static const TArray<FEVDatabaseColumnDefinition>& GetColumns();

    /** Target normalized schema introduced by the staged database refactor. */
    static int32 GetTargetSchemaVersion();
    static const TArray<FEVVocabularySchemaTable>& GetNormalizedTables();
    static const FEVVocabularySchemaTable* FindNormalizedTable(const FString& TableName);

    /** Flat user-facing exchange format. It intentionally does not mirror normalized tables. */
    static TArray<FEVDatabaseColumnDefinition> GetImportExportColumns();
    static TArray<FString> GetImportExportColumnNames();
};
