#include "EVVocabularySqlQueries.h"

#include "EVVocabularyDatabaseSchema.h"

TArray<FString> FEVVocabularySqlQueries::GetCreateNormalizedSchemaQueries()
{
    TArray<FString> Queries;
    Queries.Add(TEXT("PRAGMA foreign_keys = ON;"));

    for (const FEVVocabularySchemaTable& Table : FEVVocabularyDatabaseSchema::GetNormalizedTables())
    {
        TArray<FString> Definitions;
        Definitions.Reserve(Table.Columns.Num() + Table.Constraints.Num());

        for (const FEVVocabularySchemaColumn& Column : Table.Columns)
        {
            Definitions.Add(Column.Name + TEXT(" ") + Column.SqlDefinition);
        }

        Definitions.Append(Table.Constraints);
        Queries.Add(FString::Printf(TEXT("CREATE TABLE IF NOT EXISTS %s (%s);"), *Table.Name,
                                    *FString::Join(Definitions, TEXT(", "))));
        Queries.Append(Table.IndexQueries);
    }

    return Queries;
}

FString FEVVocabularySqlQueries::GetSelectCompatibilityColumnsQuery(const FString& WhereClause,
                                                                    const FString& OrderAndLimitClause)
{
    FString Query =
        TEXT("SELECT "
             "e.Word, "
             "COALESCE((SELECT p.Transcription FROM VocabularyPronunciations p "
             "          WHERE p.EntryId = e.Id ORDER BY p.IsPrimary DESC, p.DisplayOrder ASC, p.Id ASC LIMIT 1), ''), "
             "COALESCE((SELECT GROUP_CONCAT(d.DefinitionText, char(10)) "
             "          FROM VocabularyDefinitions d JOIN VocabularyMeanings m ON m.Id = d.MeaningId "
             "          WHERE m.EntryId = e.Id ORDER BY m.DisplayOrder, d.DisplayOrder, d.Id), ''), "
             "COALESCE((SELECT GROUP_CONCAT(d.UsageExample, char(10)) "
             "          FROM VocabularyDefinitions d JOIN VocabularyMeanings m ON m.Id = d.MeaningId "
             "          WHERE m.EntryId = e.Id AND d.UsageExample IS NOT NULL AND d.UsageExample <> '' "
             "          ORDER BY m.DisplayOrder, d.DisplayOrder, d.Id), ''), "
             "COALESCE((SELECT GROUP_CONCAT(t.TranslationText, ', ') FROM VocabularyTranslations t "
             "          WHERE t.EntryId = e.Id AND lower(t.TargetLanguage) = 'ru' "
             "          ORDER BY t.DisplayOrder, t.Id), ''), "
             "COALESCE((SELECT GROUP_CONCAT(t.TranslationText, ', ') FROM VocabularyTranslations t "
             "          WHERE t.EntryId = e.Id AND lower(t.TargetLanguage) IN ('uk', 'ua') "
             "          ORDER BY t.DisplayOrder, t.Id), '') "
             "FROM VocabularyEntries e");

    if (!WhereClause.IsEmpty())
    {
        Query += TEXT(" ") + WhereClause;
    }
    if (!OrderAndLimitClause.IsEmpty())
    {
        Query += TEXT(" ") + OrderAndLimitClause;
    }
    Query += TEXT(";");
    return Query;
}

FString FEVVocabularySqlQueries::GetVocabularyEntryByWordQuery()
{
    return GetSelectCompatibilityColumnsQuery(TEXT("WHERE e.NormalizedWord = ?"));
}

FString FEVVocabularySqlQueries::GetSelectImportExportColumnsQuery()
{
    return GetSelectCompatibilityColumnsQuery(FString(), TEXT("ORDER BY e.Word COLLATE NOCASE ASC"));
}

FString FEVVocabularySqlQueries::GetSelectVocabularyEntriesQuery()
{
    return GetSelectCompatibilityColumnsQuery(FString(), TEXT("ORDER BY e.Word COLLATE NOCASE ASC"));
}

FString FEVVocabularySqlQueries::GetSelectVocabularyEntriesPageQuery()
{
    return GetSelectCompatibilityColumnsQuery(FString(), TEXT("ORDER BY e.Word COLLATE NOCASE ASC LIMIT ? OFFSET ?"));
}

FString FEVVocabularySqlQueries::GetSelectVocabularyEntriesPageByPrefixQuery()
{
    return GetSelectCompatibilityColumnsQuery(TEXT("WHERE e.NormalizedWord LIKE ? COLLATE NOCASE"),
                                              TEXT("ORDER BY e.Word COLLATE NOCASE ASC LIMIT ? OFFSET ?"));
}

FString FEVVocabularySqlQueries::GetRandomlySelectedWordQuery()
{
    return TEXT("SELECT Word FROM VocabularyEntries ORDER BY RANDOM() LIMIT 1;");
}
