#include "EVVocabularyDatabaseSchema.h"

namespace
{
FEVVocabularySchemaTable MakeEntriesTable()
{
    FEVVocabularySchemaTable Table;
    Table.Name = TEXT("VocabularyEntries");
    Table.Columns = {
        {TEXT("Id"), TEXT("INTEGER PRIMARY KEY AUTOINCREMENT")},
        {TEXT("Word"), TEXT("TEXT NOT NULL")},
        {TEXT("NormalizedWord"), TEXT("TEXT NOT NULL")},
        {TEXT("CreatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP")},
        {TEXT("UpdatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP")},
    };
    Table.IndexQueries = {
        TEXT("CREATE UNIQUE INDEX IF NOT EXISTS UX_VocabularyEntries_NormalizedWord "
             "ON VocabularyEntries(NormalizedWord);"),
        TEXT("CREATE INDEX IF NOT EXISTS IX_VocabularyEntries_Word "
             "ON VocabularyEntries(Word COLLATE NOCASE);"),
    };
    return Table;
}

FEVVocabularySchemaTable MakePronunciationsTable()
{
    FEVVocabularySchemaTable Table;
    Table.Name = TEXT("VocabularyPronunciations");
    Table.Columns = {
        {TEXT("Id"), TEXT("INTEGER PRIMARY KEY AUTOINCREMENT")},
        {TEXT("EntryId"), TEXT("INTEGER NOT NULL")},
        {TEXT("LanguageCode"), TEXT("TEXT NOT NULL")},
        {TEXT("Transcription"), TEXT("TEXT")},
        {TEXT("AudioUrl"), TEXT("TEXT")},
        {TEXT("Accent"), TEXT("TEXT")},
        {TEXT("SourceUrl"), TEXT("TEXT")},
        {TEXT("LicenseName"), TEXT("TEXT")},
        {TEXT("LicenseUrl"), TEXT("TEXT")},
        {TEXT("IsPrimary"), TEXT("INTEGER NOT NULL DEFAULT 0")},
        {TEXT("DisplayOrder"), TEXT("INTEGER NOT NULL DEFAULT 0")},
        {TEXT("ProviderName"), TEXT("TEXT")},
        {TEXT("ProviderKey"), TEXT("TEXT")},
        {TEXT("MetadataJson"), TEXT("TEXT")},
        {TEXT("CreatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP")},
        {TEXT("UpdatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP")},
    };
    Table.Constraints = {
        TEXT("FOREIGN KEY (EntryId) REFERENCES VocabularyEntries(Id) ON DELETE CASCADE"),
    };
    Table.IndexQueries = {
        TEXT("CREATE INDEX IF NOT EXISTS IX_VocabularyPronunciations_EntryId "
             "ON VocabularyPronunciations(EntryId, DisplayOrder);"),
    };
    return Table;
}

FEVVocabularySchemaTable MakeMeaningsTable()
{
    FEVVocabularySchemaTable Table;
    Table.Name = TEXT("VocabularyMeanings");
    Table.Columns = {
        {TEXT("Id"), TEXT("INTEGER PRIMARY KEY AUTOINCREMENT")},
        {TEXT("EntryId"), TEXT("INTEGER NOT NULL")},
        {TEXT("PartOfSpeech"), TEXT("TEXT NOT NULL DEFAULT 'unspecified'")},
        {TEXT("DisplayOrder"), TEXT("INTEGER NOT NULL DEFAULT 0")},
        {TEXT("ProviderName"), TEXT("TEXT")},
        {TEXT("ProviderKey"), TEXT("TEXT")},
        {TEXT("MetadataJson"), TEXT("TEXT")},
        {TEXT("CreatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP")},
        {TEXT("UpdatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP")},
    };
    Table.Constraints = {
        TEXT("FOREIGN KEY (EntryId) REFERENCES VocabularyEntries(Id) ON DELETE CASCADE"),
    };
    // PartOfSpeech is deliberately not unique. A word can have several senses
    // that share the same grammatical category.
    Table.IndexQueries = {
        TEXT("CREATE INDEX IF NOT EXISTS IX_VocabularyMeanings_EntryId "
             "ON VocabularyMeanings(EntryId, DisplayOrder);"),
        TEXT("CREATE INDEX IF NOT EXISTS IX_VocabularyMeanings_PartOfSpeech "
             "ON VocabularyMeanings(PartOfSpeech);"),
    };
    return Table;
}

FEVVocabularySchemaTable MakeDefinitionsTable()
{
    FEVVocabularySchemaTable Table;
    Table.Name = TEXT("VocabularyDefinitions");
    Table.Columns = {
        {TEXT("Id"), TEXT("INTEGER PRIMARY KEY AUTOINCREMENT")},
        {TEXT("MeaningId"), TEXT("INTEGER NOT NULL")},
        {TEXT("DefinitionText"), TEXT("TEXT NOT NULL")},
        {TEXT("UsageExample"), TEXT("TEXT")},
        {TEXT("DisplayOrder"), TEXT("INTEGER NOT NULL DEFAULT 0")},
        {TEXT("ProviderName"), TEXT("TEXT")},
        {TEXT("ProviderKey"), TEXT("TEXT")},
        {TEXT("MetadataJson"), TEXT("TEXT")},
        {TEXT("CreatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP")},
        {TEXT("UpdatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP")},
    };
    Table.Constraints = {
        TEXT("FOREIGN KEY (MeaningId) REFERENCES VocabularyMeanings(Id) ON DELETE CASCADE"),
    };
    Table.IndexQueries = {
        TEXT("CREATE INDEX IF NOT EXISTS IX_VocabularyDefinitions_MeaningId "
             "ON VocabularyDefinitions(MeaningId, DisplayOrder);"),
    };
    return Table;
}

FEVVocabularySchemaTable MakeTranslationsTable()
{
    FEVVocabularySchemaTable Table;
    Table.Name = TEXT("VocabularyTranslations");
    Table.Columns = {
        {TEXT("Id"), TEXT("INTEGER PRIMARY KEY AUTOINCREMENT")},
        {TEXT("EntryId"), TEXT("INTEGER NOT NULL")},
        {TEXT("MeaningId"), TEXT("INTEGER")},
        {TEXT("TranslationText"), TEXT("TEXT NOT NULL")},
        {TEXT("TargetLanguage"), TEXT("TEXT NOT NULL")},
        {TEXT("TargetPartOfSpeech"), TEXT("TEXT")},
        {TEXT("DisplayOrder"), TEXT("INTEGER NOT NULL DEFAULT 0")},
        {TEXT("ProviderName"), TEXT("TEXT")},
        {TEXT("ProviderKey"), TEXT("TEXT")},
        {TEXT("Confidence"), TEXT("REAL")},
        {TEXT("MetadataJson"), TEXT("TEXT")},
        {TEXT("CreatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP")},
        {TEXT("UpdatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP")},
    };
    Table.Constraints = {
        TEXT("FOREIGN KEY (EntryId) REFERENCES VocabularyEntries(Id) ON DELETE CASCADE"),
        TEXT("FOREIGN KEY (MeaningId) REFERENCES VocabularyMeanings(Id) ON DELETE CASCADE"),
    };
    Table.IndexQueries = {
        TEXT("CREATE INDEX IF NOT EXISTS IX_VocabularyTranslations_EntryId "
             "ON VocabularyTranslations(EntryId, TargetLanguage, DisplayOrder);"),
        TEXT("CREATE INDEX IF NOT EXISTS IX_VocabularyTranslations_MeaningId "
             "ON VocabularyTranslations(MeaningId, TargetLanguage, DisplayOrder);"),
    };
    return Table;
}

FEVVocabularySchemaTable MakeRelationsTable()
{
    FEVVocabularySchemaTable Table;
    Table.Name = TEXT("VocabularyRelations");
    Table.Columns = {
        {TEXT("Id"), TEXT("INTEGER PRIMARY KEY AUTOINCREMENT")},
        {TEXT("EntryId"), TEXT("INTEGER NOT NULL")},
        {TEXT("MeaningId"), TEXT("INTEGER")},
        {TEXT("RelatedWord"), TEXT("TEXT NOT NULL")},
        {TEXT("NormalizedRelatedWord"), TEXT("TEXT NOT NULL")},
        {TEXT("RelationType"), TEXT("TEXT NOT NULL")},
        {TEXT("DisplayOrder"), TEXT("INTEGER NOT NULL DEFAULT 0")},
        {TEXT("ProviderName"), TEXT("TEXT")},
        {TEXT("ProviderKey"), TEXT("TEXT")},
        {TEXT("MetadataJson"), TEXT("TEXT")},
        {TEXT("CreatedAt"), TEXT("TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP")},
    };
    Table.Constraints = {
        TEXT("FOREIGN KEY (EntryId) REFERENCES VocabularyEntries(Id) ON DELETE CASCADE"),
        TEXT("FOREIGN KEY (MeaningId) REFERENCES VocabularyMeanings(Id) ON DELETE CASCADE"),
    };
    Table.IndexQueries = {
        TEXT("CREATE INDEX IF NOT EXISTS IX_VocabularyRelations_EntryId "
             "ON VocabularyRelations(EntryId, RelationType, DisplayOrder);"),
        TEXT("CREATE INDEX IF NOT EXISTS IX_VocabularyRelations_MeaningId "
             "ON VocabularyRelations(MeaningId, RelationType, DisplayOrder);"),
        TEXT("CREATE INDEX IF NOT EXISTS IX_VocabularyRelations_NormalizedRelatedWord "
             "ON VocabularyRelations(NormalizedRelatedWord);"),
    };
    return Table;
}
} // namespace

const FString& FEVVocabularyDatabaseSchema::GetTableName()
{
    // Version-1 compatibility entry point. The storage service still calls
    // this until transactional version-2 CRUD and migration are introduced.
    static const FString TableName = TEXT("VocabularyEntries");
    return TableName;
}

const TArray<FEVDatabaseColumnDefinition>& FEVVocabularyDatabaseSchema::GetColumns()
{
    // Version-1 compatibility schema. Do not add new normalized fields here;
    // only legacy flat SQLite binding and compatibility reads consume this collection.
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

int32 FEVVocabularyDatabaseSchema::GetTargetSchemaVersion()
{
    return 3;
}

const TArray<FEVVocabularySchemaTable>& FEVVocabularyDatabaseSchema::GetNormalizedTables()
{
    static const TArray<FEVVocabularySchemaTable> Tables = {
        MakeEntriesTable(),     MakePronunciationsTable(), MakeMeaningsTable(),
        MakeDefinitionsTable(), MakeTranslationsTable(),   MakeRelationsTable(),
    };

    return Tables;
}

const FEVVocabularySchemaTable* FEVVocabularyDatabaseSchema::FindNormalizedTable(const FString& TableName)
{
    for (const FEVVocabularySchemaTable& Table : GetNormalizedTables())
    {
        if (Table.Name.Equals(TableName, ESearchCase::CaseSensitive))
        {
            return &Table;
        }
    }

    return nullptr;
}

TArray<FEVDatabaseColumnDefinition> FEVVocabularyDatabaseSchema::GetImportExportColumns()
{
    // User-facing flat exchange format. It intentionally does not mirror the
    // normalized database tables. Only Word is mandatory; every other column
    // may be omitted from an import file and is treated as empty.
    return {
        {TEXT("Word"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Required,
         TEXT("WORD - can't be empty!!!"), nullptr, false},
        {TEXT("PartOfSpeech"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("PartOfSpeech - can be empty"), nullptr, true},
        {TEXT("Transcription"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("Transcription - can be empty"), nullptr, true},
        {TEXT("AudioUrl"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("AudioUrl - can be empty"), nullptr, true},
        {TEXT("Definition"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("Definition - can be empty"), nullptr, true},
        {TEXT("Usage"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("Usage - can be empty"), nullptr, true},
        {TEXT("TranslationRu"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("TranslationRU - can be empty; separate several values with semicolons"), nullptr, true},
        {TEXT("TranslationUa"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("TranslationUA - can be empty; separate several values with semicolons"), nullptr, true},
        {TEXT("Synonyms"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("Synonyms - can be empty; separate several values with semicolons"), nullptr, true},
        {TEXT("Antonyms"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("Antonyms - can be empty; separate several values with semicolons"), nullptr, true},
    };
}

TArray<FString> FEVVocabularyDatabaseSchema::GetImportExportColumnNames()
{
    TArray<FString> Names;
    for (const FEVDatabaseColumnDefinition& Column : GetImportExportColumns())
    {
        Names.Add(Column.Name);
    }
    return Names;
}
