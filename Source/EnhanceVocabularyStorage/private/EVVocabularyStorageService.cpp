#include "EVVocabularyStorageService.h"

#include "EVVocabularyDatabaseSchema.h"
#include "EVVocabularyFieldRegistry.h"
#include "EVVocabularySqlQueries.h"
#include "EVWordInputValidator.h"

#include "EVImportValidationReportFormatter.h"
#include "EVImportValidationRules.h"

#include "Misc/Paths.h"
#include "SQLitePreparedStatement.h"
#include "EVHelpers.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Serialization/Csv/CsvParser.h"

namespace
{
FString QuoteCsvField(const FString& Value)
{
    FString EscapedValue = Value;
    EscapedValue.ReplaceInline(TEXT("\""), TEXT("\"\""));
    return FString::Printf(TEXT("\"%s\""), *EscapedValue);
}

bool ReadEntryFields(FSQLitePreparedStatement& Statement, FVocabularyEntry& OutEntry, int32 StartColumnIndex = 0)
{
    const TArray<FEVDatabaseColumnDefinition>& Fields = FEVVocabularyFieldRegistry::GetPersistentFields();
    for (int32 FieldIndex = 0; FieldIndex < Fields.Num(); ++FieldIndex)
    {
        const FEVDatabaseColumnDefinition& Field = Fields[FieldIndex];
        if (!Field.EntryMember ||
            !Statement.GetColumnValueByIndex(StartColumnIndex + FieldIndex, OutEntry.*Field.EntryMember))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to read compatibility vocabulary field '%s'"), *Field.Name);
            return false;
        }
    }

    OutEntry.NormalizedWord = FEVWordInputValidator::NormalizeWordInput(OutEntry.Word);
    OutEntry.bHasUsageExamples = EVVocabularyUsage::HasUsageExamples(OutEntry.Usage);
    return true;
}

bool ExecutePreparedInsert(FSQLiteDatabase& Database, const FString& Query,
                           const TFunctionRef<bool(FSQLitePreparedStatement&)>& Binder)
{
    FSQLitePreparedStatement Statement;
    if (!Statement.Create(Database, *Query, ESQLitePreparedStatementFlags::Persistent) || !Binder(Statement))
    {
        return false;
    }
    return Statement.Execute();
}

FString NormalizeExchangeColumnName(const FString& Value)
{
    FString Result = Value.TrimStartAndEnd();
    Result.ToLowerInline();
    Result.ReplaceInline(TEXT("_"), TEXT(""));
    Result.ReplaceInline(TEXT(" "), TEXT(""));
    return Result;
}

FString CanonicalExchangeColumnName(const FString& Value)
{
    const FString Normalized = NormalizeExchangeColumnName(Value);
    if (Normalized == TEXT("word"))
        return TEXT("Word");
    if (Normalized == TEXT("partofspeech") || Normalized == TEXT("pos"))
        return TEXT("PartOfSpeech");
    if (Normalized == TEXT("transcription") || Normalized == TEXT("phonetic"))
        return TEXT("Transcription");
    if (Normalized == TEXT("audiourl") || Normalized == TEXT("audio"))
        return TEXT("AudioUrl");
    if (Normalized == TEXT("definition"))
        return TEXT("Definition");
    if (Normalized == TEXT("usage") || Normalized == TEXT("example"))
        return TEXT("Usage");
    if (Normalized == TEXT("translationru") || Normalized == TEXT("translationrussian"))
        return TEXT("TranslationRu");
    if (Normalized == TEXT("translationua") || Normalized == TEXT("translationuk") ||
        Normalized == TEXT("translationukrainian"))
        return TEXT("TranslationUa");
    if (Normalized == TEXT("synonym") || Normalized == TEXT("synonyms"))
        return TEXT("Synonyms");
    if (Normalized == TEXT("antonym") || Normalized == TEXT("antonyms"))
        return TEXT("Antonyms");
    return FString();
}

TArray<FString> SplitExchangeValues(const FString& Source)
{
    FString Normalized = Source;
    Normalized.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
    Normalized.ReplaceInline(TEXT("\r"), TEXT("\n"));
    Normalized.ReplaceInline(TEXT(";"), TEXT("\n"));
    Normalized.ReplaceInline(TEXT(","), TEXT("\n"));

    TArray<FString> RawValues;
    Normalized.ParseIntoArray(RawValues, TEXT("\n"), true);

    TArray<FString> Values;
    for (FString Value : RawValues)
    {
        Value = Value.TrimStartAndEnd();
        if (!Value.IsEmpty() &&
            !Values.ContainsByPredicate([&Value](const FString& Existing)
                                        { return Existing.Equals(Value, ESearchCase::IgnoreCase); }))
        {
            Values.Add(MoveTemp(Value));
        }
    }
    return Values;
}

FString RemoveExchangeNumberPrefix(const FString& Source)
{
    FString Result = Source.TrimStartAndEnd();
    int32 DotIndex = INDEX_NONE;
    if (!Result.FindChar(TEXT('.'), DotIndex))
        return Result;
    const FString Prefix = Result.Left(DotIndex).TrimStartAndEnd();
    return Prefix.IsNumeric() ? Result.Mid(DotIndex + 1).TrimStartAndEnd() : Result;
}

TArray<FString> SplitDefinitionValues(const FString& Source)
{
    TArray<FString> Lines;
    FString Normalized = Source;
    Normalized.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
    Normalized.ReplaceInline(TEXT("\r"), TEXT("\n"));
    Normalized.ParseIntoArray(Lines, TEXT("\n"), true);

    TArray<FString> Values;
    for (FString Line : Lines)
    {
        Line = RemoveExchangeNumberPrefix(Line);
        if (!Line.IsEmpty())
            Values.Add(MoveTemp(Line));
    }
    if (Values.IsEmpty() && !Source.TrimStartAndEnd().IsEmpty())
    {
        Values.Add(Source.TrimStartAndEnd());
    }
    return Values;
}

TMap<int32, FString> ParseUsageValues(const FString& Source)
{
    TMap<int32, FString> Result;
    TArray<FString> Lines;
    FString Normalized = Source;
    Normalized.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
    Normalized.ReplaceInline(TEXT("\r"), TEXT("\n"));
    Normalized.ParseIntoArray(Lines, TEXT("\n"), true);

    int32 SequentialIndex = 0;
    for (FString Line : Lines)
    {
        Line = Line.TrimStartAndEnd();
        if (Line.IsEmpty() || Line.Equals(EVVocabularyUsage::GetNoUsageExamplesText(), ESearchCase::IgnoreCase) ||
            Line.Equals(TEXT("No usage was provided"), ESearchCase::IgnoreCase))
        {
            continue;
        }

        int32 DotIndex = INDEX_NONE;
        int32 DefinitionIndex = SequentialIndex;
        FString Value = Line;
        if (Line.FindChar(TEXT('.'), DotIndex))
        {
            const FString Prefix = Line.Left(DotIndex).TrimStartAndEnd();
            if (Prefix.IsNumeric() && FCString::Atoi(*Prefix) > 0)
            {
                DefinitionIndex = FCString::Atoi(*Prefix) - 1;
                Value = Line.Mid(DotIndex + 1).TrimStartAndEnd();
            }
        }
        if (!Value.IsEmpty())
            Result.Add(DefinitionIndex, MoveTemp(Value));
        ++SequentialIndex;
    }
    return Result;
}

FEVVocabularyMeaning& FindOrAddExchangeMeaning(FEVVocabularyRecord& Record, const FString& PartOfSpeech)
{
    const FString ResolvedPartOfSpeech =
        PartOfSpeech.TrimStartAndEnd().IsEmpty() ? TEXT("unspecified") : PartOfSpeech.TrimStartAndEnd();
    for (FEVVocabularyMeaning& Meaning : Record.Meanings)
    {
        if (Meaning.PartOfSpeech.Equals(ResolvedPartOfSpeech, ESearchCase::IgnoreCase))
            return Meaning;
    }
    FEVVocabularyMeaning Meaning;
    Meaning.PartOfSpeech = ResolvedPartOfSpeech;
    Meaning.DisplayOrder = Record.Meanings.Num();
    return Record.Meanings[Record.Meanings.Add(MoveTemp(Meaning))];
}

void AddUniqueTranslation(TArray<FEVVocabularyTranslation>& Target, const FString& Text, const FString& Language,
                          const FString& PartOfSpeech)
{
    const FString Trimmed = Text.TrimStartAndEnd();
    if (Trimmed.IsEmpty())
        return;
    if (Target.ContainsByPredicate(
            [&](const FEVVocabularyTranslation& Existing)
            {
                return Existing.TargetLanguage.Equals(Language, ESearchCase::IgnoreCase) &&
                       Existing.TranslationText.Equals(Trimmed, ESearchCase::IgnoreCase);
            }))
        return;
    FEVVocabularyTranslation Value;
    Value.TranslationText = Trimmed;
    Value.TargetLanguage = Language;
    Value.TargetPartOfSpeech = PartOfSpeech;
    Value.DisplayOrder = Target.Num();
    Target.Add(MoveTemp(Value));
}

void AddUniqueRelation(TArray<FEVVocabularyRelation>& Target, const FString& Word, const FString& Type)
{
    const FString Trimmed = Word.TrimStartAndEnd();
    if (Trimmed.IsEmpty())
        return;
    if (Target.ContainsByPredicate(
            [&](const FEVVocabularyRelation& Existing)
            {
                return Existing.RelationType.Equals(Type, ESearchCase::IgnoreCase) &&
                       Existing.RelatedWord.Equals(Trimmed, ESearchCase::IgnoreCase);
            }))
        return;
    FEVVocabularyRelation Value;
    Value.RelatedWord = Trimmed;
    Value.NormalizedRelatedWord = FEVWordInputValidator::NormalizeWordInput(Trimmed);
    Value.RelationType = Type;
    Value.DisplayOrder = Target.Num();
    Target.Add(MoveTemp(Value));
}

FString JoinTranslationValues(const TArray<FEVVocabularyTranslation>& Values, const FString& Language)
{
    TArray<FString> Result;
    for (const FEVVocabularyTranslation& Value : Values)
    {
        if (Value.TargetLanguage.Equals(Language, ESearchCase::IgnoreCase) && !Value.TranslationText.IsEmpty())
            Result.AddUnique(Value.TranslationText);
    }
    return FString::Join(Result, TEXT("; "));
}

FString JoinRelationValues(const TArray<FEVVocabularyRelation>& Values, const FString& Type)
{
    TArray<FString> Result;
    for (const FEVVocabularyRelation& Value : Values)
    {
        if (Value.RelationType.Equals(Type, ESearchCase::IgnoreCase) && !Value.RelatedWord.IsEmpty())
            Result.AddUnique(Value.RelatedWord);
    }
    return FString::Join(Result, TEXT("; "));
}
} // namespace

bool UEVVocabularyStorageService::InitializeStorage()
{
#if WITH_EDITOR
    const FString DbPath = UEVHelpers::GetVocabularyDebugDbPath();
    UE_LOG(LogTemp, Warning, TEXT("Editor Debug DB path: %s"), *DbPath);
#else
    const FString DbPath = UEVHelpers::GetVocabularyLiveDbPath();
    UE_LOG(LogTemp, Warning, TEXT("Packaged Live DB path: %s"), *DbPath);
#endif

    IFileManager::Get().MakeDirectory(*FPaths::GetPath(DbPath), true);
    if (!Database.Open(*DbPath, ESQLiteDatabaseOpenMode::ReadWriteCreate) || !Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to open/create vocabulary database"));
        return false;
    }

    if (!Database.Execute(TEXT("PRAGMA foreign_keys = ON;")))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to enable SQLite foreign keys"));
        return false;
    }

    return InitializeSchema();
}

bool UEVVocabularyStorageService::GetSchemaVersion(int32& OutVersion)
{
    OutVersion = 0;
    FSQLitePreparedStatement Statement;
    if (!Statement.Create(Database, TEXT("PRAGMA user_version;"), ESQLitePreparedStatementFlags::Persistent) ||
        Statement.Step() != ESQLitePreparedStatementStepResult::Row)
    {
        return false;
    }
    return Statement.GetColumnValueByIndex(0, OutVersion);
}

bool UEVVocabularyStorageService::SetSchemaVersion(int32 Version)
{
    return Database.Execute(*FString::Printf(TEXT("PRAGMA user_version = %d;"), Version));
}

bool UEVVocabularyStorageService::IsLegacySchema(bool& bOutIsLegacy)
{
    bOutIsLegacy = false;
    FSQLitePreparedStatement TableStatement;
    if (!TableStatement.Create(
            Database, TEXT("SELECT 1 FROM sqlite_master WHERE type='table' AND name='VocabularyEntries' LIMIT 1;"),
            ESQLitePreparedStatementFlags::Persistent))
    {
        return false;
    }
    if (TableStatement.Step() != ESQLitePreparedStatementStepResult::Row)
    {
        return true;
    }

    FSQLitePreparedStatement ColumnStatement;
    if (!ColumnStatement.Create(Database, TEXT("PRAGMA table_info(VocabularyEntries);"),
                                ESQLitePreparedStatementFlags::Persistent))
    {
        return false;
    }

    bool bHasNormalizedWord = false;
    bool bHasLegacyDefinition = false;
    while (ColumnStatement.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        FString ColumnName;
        if (ColumnStatement.GetColumnValueByIndex(1, ColumnName))
        {
            bHasNormalizedWord |= ColumnName == TEXT("NormalizedWord");
            bHasLegacyDefinition |= ColumnName == TEXT("Definition");
        }
    }
    bOutIsLegacy = bHasLegacyDefinition && !bHasNormalizedWord;
    return true;
}

bool UEVVocabularyStorageService::CreateNormalizedSchema()
{
    for (const FString& Query : FEVVocabularySqlQueries::GetCreateNormalizedSchemaQueries())
    {
        if (!Database.Execute(*Query))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed normalized schema query: %s"), *Query);
            return false;
        }
    }
    return SetSchemaVersion(FEVVocabularyDatabaseSchema::GetTargetSchemaVersion());
}

bool UEVVocabularyStorageService::InitializeSchema()
{
    int32 Version = 0;
    if (!GetSchemaVersion(Version))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to read vocabulary schema version"));
        return false;
    }

    bool bLegacy = false;
    if (!IsLegacySchema(bLegacy))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to inspect vocabulary schema"));
        return false;
    }

    if (bLegacy)
    {
        return MigrateLegacySchema();
    }

    if (Version > 0 && Version < FEVVocabularyDatabaseSchema::GetTargetSchemaVersion() &&
        !MigrateNormalizedSchemaToVersion3(Version))
    {
        return false;
    }

    if (!CreateNormalizedSchema())
    {
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("Vocabulary schema ready. Version=%d"),
           FEVVocabularyDatabaseSchema::GetTargetSchemaVersion());
    return true;
}

bool UEVVocabularyStorageService::MigrateNormalizedSchemaToVersion3(const int32 CurrentVersion)
{
    if (CurrentVersion >= 3)
    {
        return true;
    }

    if (!Database.Execute(TEXT("BEGIN IMMEDIATE TRANSACTION;")))
    {
        return false;
    }

    auto Rollback = [this]()
    {
        Database.Execute(TEXT("ROLLBACK;"));
        return false;
    };

    bool bHasLanguageCode = false;
    FSQLitePreparedStatement ColumnStatement;
    if (!ColumnStatement.Create(Database, TEXT("PRAGMA table_info(VocabularyPronunciations);"),
                                ESQLitePreparedStatementFlags::Persistent))
    {
        return Rollback();
    }

    while (ColumnStatement.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        FString ColumnName;
        if (ColumnStatement.GetColumnValueByIndex(1, ColumnName) && ColumnName == TEXT("LanguageCode"))
        {
            bHasLanguageCode = true;
            break;
        }
    }

    if (!bHasLanguageCode &&
        !Database.Execute(
            TEXT("ALTER TABLE VocabularyPronunciations ADD COLUMN LanguageCode TEXT NOT NULL DEFAULT 'en';")))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to add LanguageCode to VocabularyPronunciations."));
        return Rollback();
    }

    if (!Database.Execute(TEXT("UPDATE VocabularyPronunciations SET LanguageCode = 'en' WHERE LanguageCode IS NULL OR "
                               "trim(LanguageCode) = '';")) ||
        !SetSchemaVersion(3) || !Database.Execute(TEXT("COMMIT;")))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to complete pronunciation language-code migration."));
        return Rollback();
    }

    return true;
}

bool UEVVocabularyStorageService::MigrateLegacySchema()
{
    UE_LOG(LogTemp, Warning, TEXT("Migrating legacy vocabulary schema to version 3"));
    if (!Database.Execute(TEXT("BEGIN IMMEDIATE TRANSACTION;")))
    {
        return false;
    }

    auto Rollback = [this]()
    {
        Database.Execute(TEXT("ROLLBACK;"));
        return false;
    };

    if (!Database.Execute(TEXT("ALTER TABLE VocabularyEntries RENAME TO VocabularyEntries_Legacy;")))
    {
        return Rollback();
    }

    for (const FString& Query : FEVVocabularySqlQueries::GetCreateNormalizedSchemaQueries())
    {
        if (!Database.Execute(*Query))
        {
            return Rollback();
        }
    }

    FSQLitePreparedStatement LegacyStatement;
    if (!LegacyStatement.Create(Database,
                                TEXT("SELECT Word, Transcription, Definition, Usage, TranslationRu, TranslationUa "
                                     "FROM VocabularyEntries_Legacy ORDER BY Id;"),
                                ESQLitePreparedStatementFlags::Persistent))
    {
        return Rollback();
    }

    while (LegacyStatement.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        FVocabularyEntry Entry;
        if (!LegacyStatement.GetColumnValueByIndex(0, Entry.Word) ||
            !LegacyStatement.GetColumnValueByIndex(1, Entry.Transcription) ||
            !LegacyStatement.GetColumnValueByIndex(2, Entry.Definition) ||
            !LegacyStatement.GetColumnValueByIndex(3, Entry.Usage) ||
            !LegacyStatement.GetColumnValueByIndex(4, Entry.TranslationRu) ||
            !LegacyStatement.GetColumnValueByIndex(5, Entry.TranslationUa))
        {
            return Rollback();
        }

        if (!SaveVocabularyRecordInternal(ConvertLegacyEntryToRecord(Entry), false))
        {
            return Rollback();
        }
    }

    if (!Database.Execute(TEXT("DROP TABLE VocabularyEntries_Legacy;")) ||
        !SetSchemaVersion(FEVVocabularyDatabaseSchema::GetTargetSchemaVersion()) || !Database.Execute(TEXT("COMMIT;")))
    {
        return Rollback();
    }

    UE_LOG(LogTemp, Warning, TEXT("Legacy vocabulary migration completed"));
    return true;
}

FEVVocabularyRecord UEVVocabularyStorageService::ConvertLegacyEntryToRecord(const FVocabularyEntry& Entry)
{
    FEVVocabularyRecord Record;
    Record.NormalizedWord =
        FEVWordInputValidator::NormalizeWordInput(Entry.NormalizedWord.IsEmpty() ? Entry.Word : Entry.NormalizedWord);

    // Current casing policy: presentation and lookup values are identical.
    // The separate columns remain ready for a future casing-correction layer.
    Record.Word = Record.NormalizedWord;

    if (!Entry.Transcription.IsEmpty())
    {
        FEVVocabularyPronunciation Pronunciation;
        Pronunciation.LanguageCode = TEXT("en");
        Pronunciation.Transcription = Entry.Transcription;
        Pronunciation.bPrimary = true;
        Record.Pronunciations.Add(MoveTemp(Pronunciation));
    }

    if (!Entry.Definition.IsEmpty() || EVVocabularyUsage::HasUsageExamples(Entry.Usage))
    {
        FEVVocabularyMeaning Meaning;
        Meaning.PartOfSpeech = TEXT("unspecified");
        FEVVocabularyDefinition Definition;
        Definition.DefinitionText = Entry.Definition;
        Definition.UsageExample = EVVocabularyUsage::HasUsageExamples(Entry.Usage) ? Entry.Usage : FString();
        Meaning.Definitions.Add(MoveTemp(Definition));
        Record.Meanings.Add(MoveTemp(Meaning));
    }

    auto AddTranslation = [&Record](const FString& Text, const TCHAR* Language)
    {
        if (!Text.IsEmpty())
        {
            FEVVocabularyTranslation Translation;
            Translation.TranslationText = Text;
            Translation.TargetLanguage = Language;
            Record.GeneralTranslations.Add(MoveTemp(Translation));
        }
    };
    AddTranslation(Entry.TranslationRu, TEXT("ru"));
    AddTranslation(Entry.TranslationUa, TEXT("uk"));
    return Record;
}

FVocabularyEntry UEVVocabularyStorageService::FlattenRecord(const FEVVocabularyRecord& Record)
{
    FVocabularyEntry Entry;
    Entry.Word = Record.Word;
    Entry.NormalizedWord = Record.NormalizedWord;
    if (!Record.Pronunciations.IsEmpty())
    {
        const FEVVocabularyPronunciation* Primary =
            Record.Pronunciations.FindByPredicate([](const FEVVocabularyPronunciation& Item) { return Item.bPrimary; });
        Entry.Transcription = (Primary ? Primary : &Record.Pronunciations[0])->Transcription;
    }

    TArray<FString> Definitions;
    TArray<FString> Usages;
    for (const FEVVocabularyMeaning& Meaning : Record.Meanings)
    {
        for (const FEVVocabularyDefinition& Definition : Meaning.Definitions)
        {
            if (!Definition.DefinitionText.IsEmpty())
                Definitions.Add(Definition.DefinitionText);
            if (!Definition.UsageExample.IsEmpty())
                Usages.Add(Definition.UsageExample);
        }
    }
    Entry.Definition = FString::Join(Definitions, TEXT("\n"));
    Entry.Usage = Usages.IsEmpty() ? EVVocabularyUsage::GetNoUsageExamplesText() : FString::Join(Usages, TEXT("\n"));

    TArray<FString> Ru;
    TArray<FString> Uk;
    auto Collect = [&Ru, &Uk](const FEVVocabularyTranslation& Translation)
    {
        if (Translation.TargetLanguage.Equals(TEXT("ru"), ESearchCase::IgnoreCase))
            Ru.Add(Translation.TranslationText);
        else if (Translation.TargetLanguage.Equals(TEXT("uk"), ESearchCase::IgnoreCase) ||
                 Translation.TargetLanguage.Equals(TEXT("ua"), ESearchCase::IgnoreCase))
            Uk.Add(Translation.TranslationText);
    };
    for (const auto& Translation : Record.GeneralTranslations)
        Collect(Translation);
    for (const auto& Meaning : Record.Meanings)
        for (const auto& Translation : Meaning.Translations)
            Collect(Translation);
    Entry.TranslationRu = FString::Join(Ru, TEXT(", "));
    Entry.TranslationUa = FString::Join(Uk, TEXT(", "));
    Entry.bHasUsageExamples = EVVocabularyUsage::HasUsageExamples(Entry.Usage);
    return Entry;
}

bool UEVVocabularyStorageService::GetLastInsertedRowId(int64& OutRowId)
{
    OutRowId = 0;
    FSQLitePreparedStatement Statement;
    if (!Statement.Create(Database, TEXT("SELECT last_insert_rowid();"), ESQLitePreparedStatementFlags::Persistent) ||
        Statement.Step() != ESQLitePreparedStatementStepResult::Row)
    {
        return false;
    }
    return Statement.GetColumnValueByIndex(0, OutRowId);
}

bool UEVVocabularyStorageService::GetEntryIdByNormalizedWord(const FString& NormalizedWord, int64& OutEntryId)
{
    OutEntryId = 0;
    FSQLitePreparedStatement Statement;
    if (!Statement.Create(Database, TEXT("SELECT Id FROM VocabularyEntries WHERE NormalizedWord = ? LIMIT 1;"),
                          ESQLitePreparedStatementFlags::Persistent) ||
        !Statement.SetBindingValueByIndex(1, FEVWordInputValidator::NormalizeWordInput(NormalizedWord)) ||
        Statement.Step() != ESQLitePreparedStatementStepResult::Row)
    {
        return false;
    }
    return Statement.GetColumnValueByIndex(0, OutEntryId);
}

bool UEVVocabularyStorageService::InsertRecordChildren(int64 EntryId, const FEVVocabularyRecord& Record)
{
    for (const FEVVocabularyPronunciation& P : Record.Pronunciations)
    {
        const FString Query = TEXT("INSERT INTO VocabularyPronunciations "
                                   "(EntryId, LanguageCode, Transcription, AudioUrl, Accent, SourceUrl, LicenseName, "
                                   "LicenseUrl, IsPrimary, DisplayOrder, ProviderName, ProviderKey) "
                                   "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
        if (!ExecutePreparedInsert(
                Database, Query,
                [&](FSQLitePreparedStatement& S)
                {
                    return S.SetBindingValueByIndex(1, EntryId) && S.SetBindingValueByIndex(2, P.LanguageCode) &&
                           S.SetBindingValueByIndex(3, P.Transcription) && S.SetBindingValueByIndex(4, P.AudioUrl) &&
                           S.SetBindingValueByIndex(5, P.Accent) && S.SetBindingValueByIndex(6, P.SourceUrl) &&
                           S.SetBindingValueByIndex(7, P.LicenseName) && S.SetBindingValueByIndex(8, P.LicenseUrl) &&
                           S.SetBindingValueByIndex(9, P.bPrimary ? 1 : 0) &&
                           S.SetBindingValueByIndex(10, P.DisplayOrder) &&
                           S.SetBindingValueByIndex(11, P.ProviderName) && S.SetBindingValueByIndex(12, P.ProviderKey);
                }))
            return false;
    }

    auto InsertTranslation = [&](const FEVVocabularyTranslation& T, int64 MeaningId) -> bool
    {
        const FString Query = TEXT("INSERT INTO VocabularyTranslations "
                                   "(EntryId, MeaningId, TranslationText, TargetLanguage, TargetPartOfSpeech, "
                                   "DisplayOrder, ProviderName, ProviderKey, Confidence) "
                                   "VALUES (?, NULLIF(?, 0), ?, ?, ?, ?, ?, ?, ?);");
        return ExecutePreparedInsert(
            Database, Query,
            [&](FSQLitePreparedStatement& S)
            {
                return S.SetBindingValueByIndex(1, EntryId) && S.SetBindingValueByIndex(2, MeaningId) &&
                       S.SetBindingValueByIndex(3, T.TranslationText) &&
                       S.SetBindingValueByIndex(4, T.TargetLanguage) &&
                       S.SetBindingValueByIndex(5, T.TargetPartOfSpeech) &&
                       S.SetBindingValueByIndex(6, T.DisplayOrder) && S.SetBindingValueByIndex(7, T.ProviderName) &&
                       S.SetBindingValueByIndex(8, T.ProviderKey) && S.SetBindingValueByIndex(9, T.Confidence);
            });
    };

    for (const FEVVocabularyTranslation& T : Record.GeneralTranslations)
    {
        if (!InsertTranslation(T, 0))
            return false;
    }

    for (const FEVVocabularyMeaning& M : Record.Meanings)
    {
        const FString MeaningQuery =
            TEXT("INSERT INTO VocabularyMeanings "
                 "(EntryId, PartOfSpeech, DisplayOrder, ProviderName, ProviderKey) VALUES (?, ?, ?, ?, ?);");
        if (!ExecutePreparedInsert(Database, MeaningQuery,
                                   [&](FSQLitePreparedStatement& S)
                                   {
                                       return S.SetBindingValueByIndex(1, EntryId) &&
                                              S.SetBindingValueByIndex(2, M.PartOfSpeech) &&
                                              S.SetBindingValueByIndex(3, M.DisplayOrder) &&
                                              S.SetBindingValueByIndex(4, M.ProviderName) &&
                                              S.SetBindingValueByIndex(5, M.ProviderKey);
                                   }))
            return false;

        int64 MeaningId = 0;
        if (!GetLastInsertedRowId(MeaningId))
            return false;

        for (const FEVVocabularyDefinition& D : M.Definitions)
        {
            if (D.DefinitionText.IsEmpty() && D.UsageExample.IsEmpty())
                continue;
            const FString Query = TEXT("INSERT INTO VocabularyDefinitions "
                                       "(MeaningId, DefinitionText, UsageExample, DisplayOrder, ProviderName, "
                                       "ProviderKey) VALUES (?, ?, ?, ?, ?, ?);");
            if (!ExecutePreparedInsert(Database, Query,
                                       [&](FSQLitePreparedStatement& S)
                                       {
                                           return S.SetBindingValueByIndex(1, MeaningId) &&
                                                  S.SetBindingValueByIndex(2, D.DefinitionText) &&
                                                  S.SetBindingValueByIndex(3, D.UsageExample) &&
                                                  S.SetBindingValueByIndex(4, D.DisplayOrder) &&
                                                  S.SetBindingValueByIndex(5, D.ProviderName) &&
                                                  S.SetBindingValueByIndex(6, D.ProviderKey);
                                       }))
                return false;
        }

        for (const FEVVocabularyTranslation& T : M.Translations)
        {
            if (!InsertTranslation(T, MeaningId))
                return false;
        }

        for (const FEVVocabularyRelation& R : M.Relations)
        {
            const FString NormalizedRelated = R.NormalizedRelatedWord.IsEmpty()
                                                  ? FEVWordInputValidator::NormalizeWordInput(R.RelatedWord)
                                                  : R.NormalizedRelatedWord;
            const FString Query = TEXT("INSERT INTO VocabularyRelations "
                                       "(EntryId, MeaningId, RelatedWord, NormalizedRelatedWord, RelationType, "
                                       "DisplayOrder, ProviderName, ProviderKey) "
                                       "VALUES (?, ?, ?, ?, ?, ?, ?, ?);");
            if (!ExecutePreparedInsert(Database, Query,
                                       [&](FSQLitePreparedStatement& S)
                                       {
                                           return S.SetBindingValueByIndex(1, EntryId) &&
                                                  S.SetBindingValueByIndex(2, MeaningId) &&
                                                  S.SetBindingValueByIndex(3, R.RelatedWord) &&
                                                  S.SetBindingValueByIndex(4, NormalizedRelated) &&
                                                  S.SetBindingValueByIndex(5, R.RelationType) &&
                                                  S.SetBindingValueByIndex(6, R.DisplayOrder) &&
                                                  S.SetBindingValueByIndex(7, R.ProviderName) &&
                                                  S.SetBindingValueByIndex(8, R.ProviderKey);
                                       }))
                return false;
        }
    }
    return true;
}

bool UEVVocabularyStorageService::SaveVocabularyRecordInternal(const FEVVocabularyRecord& InputRecord,
                                                               bool bManageTransaction)
{
    FEVVocabularyRecord Record = InputRecord;
    Record.NormalizedWord = FEVWordInputValidator::NormalizeWordInput(
        Record.NormalizedWord.IsEmpty() ? Record.Word : Record.NormalizedWord);
    if (Record.Word.IsEmpty())
        Record.Word = Record.NormalizedWord;
    if (Record.NormalizedWord.IsEmpty())
        return false;

    if (bManageTransaction && !Database.Execute(TEXT("BEGIN IMMEDIATE TRANSACTION;")))
        return false;
    auto Fail = [this, bManageTransaction]()
    {
        if (bManageTransaction)
            Database.Execute(TEXT("ROLLBACK;"));
        return false;
    };

    const FString Query = TEXT("INSERT INTO VocabularyEntries (Word, NormalizedWord) VALUES (?, ?);");
    if (!ExecutePreparedInsert(Database, Query,
                               [&](FSQLitePreparedStatement& S) {
                                   return S.SetBindingValueByIndex(1, Record.Word) &&
                                          S.SetBindingValueByIndex(2, Record.NormalizedWord);
                               }))
        return Fail();

    int64 EntryId = 0;
    if (!GetLastInsertedRowId(EntryId) || !InsertRecordChildren(EntryId, Record))
        return Fail();
    if (bManageTransaction && !Database.Execute(TEXT("COMMIT;")))
        return Fail();
    return true;
}

bool UEVVocabularyStorageService::ReplaceVocabularyRecordInternal(const FEVVocabularyRecord& InputRecord,
                                                                  bool bManageTransaction)
{
    FEVVocabularyRecord Record = InputRecord;
    Record.NormalizedWord = FEVWordInputValidator::NormalizeWordInput(
        Record.NormalizedWord.IsEmpty() ? Record.Word : Record.NormalizedWord);
    if (Record.Word.IsEmpty())
        Record.Word = Record.NormalizedWord;

    int64 EntryId = 0;
    if (!GetEntryIdByNormalizedWord(Record.NormalizedWord, EntryId))
        return false;
    if (bManageTransaction && !Database.Execute(TEXT("BEGIN IMMEDIATE TRANSACTION;")))
        return false;
    auto Fail = [this, bManageTransaction]()
    {
        if (bManageTransaction)
            Database.Execute(TEXT("ROLLBACK;"));
        return false;
    };

    if (!ExecutePreparedInsert(
            Database, TEXT("UPDATE VocabularyEntries SET Word = ?, UpdatedAt = CURRENT_TIMESTAMP WHERE Id = ?;"),
            [&](FSQLitePreparedStatement& S)
            { return S.SetBindingValueByIndex(1, Record.Word) && S.SetBindingValueByIndex(2, EntryId); }))
        return Fail();

    if (!Database.Execute(
            *FString::Printf(TEXT("DELETE FROM VocabularyPronunciations WHERE EntryId = %lld;"), EntryId)) ||
        !Database.Execute(
            *FString::Printf(TEXT("DELETE FROM VocabularyTranslations WHERE EntryId = %lld;"), EntryId)) ||
        !Database.Execute(*FString::Printf(TEXT("DELETE FROM VocabularyRelations WHERE EntryId = %lld;"), EntryId)) ||
        !Database.Execute(*FString::Printf(TEXT("DELETE FROM VocabularyMeanings WHERE EntryId = %lld;"), EntryId)) ||
        !InsertRecordChildren(EntryId, Record))
        return Fail();

    if (bManageTransaction && !Database.Execute(TEXT("COMMIT;")))
        return Fail();
    return true;
}

bool UEVVocabularyStorageService::SaveVocabularyRecord(const FEVVocabularyRecord& Record)
{
    return Database.IsValid() && SaveVocabularyRecordInternal(Record, true);
}

bool UEVVocabularyStorageService::UpdateVocabularyRecord(const FEVVocabularyRecord& Record)
{
    return Database.IsValid() && ReplaceVocabularyRecordInternal(Record, true);
}

bool UEVVocabularyStorageService::DeleteVocabularyRecord(const FString& NormalizedWord)
{
    if (!Database.IsValid())
        return false;
    FSQLitePreparedStatement Statement;
    return Statement.Create(Database, FEVVocabularySqlQueries::DeleteVocabularyEntry,
                            ESQLitePreparedStatementFlags::Persistent) &&
           Statement.SetBindingValueByIndex(1, FEVWordInputValidator::NormalizeWordInput(NormalizedWord)) &&
           Statement.Execute();
}

bool UEVVocabularyStorageService::SaveVocabularyEntry(const FVocabularyEntry& Entry)
{
    return SaveVocabularyRecord(ConvertLegacyEntryToRecord(Entry));
}

bool UEVVocabularyStorageService::UpdateVocabularyEntry(const FVocabularyEntry& Entry)
{
    return UpdateVocabularyRecord(ConvertLegacyEntryToRecord(Entry));
}

bool UEVVocabularyStorageService::DeleteVocabularyEntry(const FVocabularyEntry& Entry)
{
    return DeleteVocabularyRecord(Entry.NormalizedWord.IsEmpty() ? Entry.Word : Entry.NormalizedWord);
}

bool UEVVocabularyStorageService::GetVocabularyEntryByWord(const FString& Word, FVocabularyEntry& OutEntry)
{
    if (!Database.IsValid())
        return false;
    FSQLitePreparedStatement Statement;
    if (!Statement.Create(Database, *FEVVocabularySqlQueries::GetVocabularyEntryByWordQuery(),
                          ESQLitePreparedStatementFlags::Persistent) ||
        !Statement.SetBindingValueByIndex(1, FEVWordInputValidator::NormalizeWordInput(Word)) ||
        Statement.Step() != ESQLitePreparedStatementStepResult::Row)
    {
        return false;
    }
    return ReadEntryFields(Statement, OutEntry);
}

bool UEVVocabularyStorageService::GetVocabularyRecordByWord(const FString& Word, FEVVocabularyRecord& OutRecord)
{
    OutRecord = FEVVocabularyRecord();
    const FString Normalized = FEVWordInputValidator::NormalizeWordInput(Word);
    int64 EntryId = 0;
    if (!Database.IsValid() || !GetEntryIdByNormalizedWord(Normalized, EntryId))
        return false;

    FSQLitePreparedStatement Root;
    if (!Root.Create(Database, TEXT("SELECT Word, NormalizedWord FROM VocabularyEntries WHERE Id = ?;"),
                     ESQLitePreparedStatementFlags::Persistent) ||
        !Root.SetBindingValueByIndex(1, EntryId) || Root.Step() != ESQLitePreparedStatementStepResult::Row ||
        !Root.GetColumnValueByIndex(0, OutRecord.Word) || !Root.GetColumnValueByIndex(1, OutRecord.NormalizedWord))
        return false;

    FSQLitePreparedStatement P;
    if (!P.Create(Database,
                  TEXT("SELECT LanguageCode, Transcription, AudioUrl, Accent, SourceUrl, LicenseName, LicenseUrl, "
                       "IsPrimary, DisplayOrder, ProviderName, ProviderKey FROM VocabularyPronunciations WHERE EntryId "
                       "= ? ORDER BY DisplayOrder, Id;"),
                  ESQLitePreparedStatementFlags::Persistent) ||
        !P.SetBindingValueByIndex(1, EntryId))
        return false;
    while (P.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        FEVVocabularyPronunciation V;
        int32 Primary = 0;
        P.GetColumnValueByIndex(0, V.LanguageCode);
        P.GetColumnValueByIndex(1, V.Transcription);
        P.GetColumnValueByIndex(2, V.AudioUrl);
        P.GetColumnValueByIndex(3, V.Accent);
        P.GetColumnValueByIndex(4, V.SourceUrl);
        P.GetColumnValueByIndex(5, V.LicenseName);
        P.GetColumnValueByIndex(6, V.LicenseUrl);
        P.GetColumnValueByIndex(7, Primary);
        V.bPrimary = Primary != 0;
        P.GetColumnValueByIndex(8, V.DisplayOrder);
        P.GetColumnValueByIndex(9, V.ProviderName);
        P.GetColumnValueByIndex(10, V.ProviderKey);
        OutRecord.Pronunciations.Add(MoveTemp(V));
    }

    TMap<int64, int32> MeaningIndex;
    FSQLitePreparedStatement M;
    if (!M.Create(Database,
                  TEXT("SELECT Id, PartOfSpeech, DisplayOrder, ProviderName, ProviderKey FROM VocabularyMeanings WHERE "
                       "EntryId = ? ORDER BY DisplayOrder, Id;"),
                  ESQLitePreparedStatementFlags::Persistent) ||
        !M.SetBindingValueByIndex(1, EntryId))
        return false;
    while (M.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        int64 MeaningId = 0;
        FEVVocabularyMeaning V;
        M.GetColumnValueByIndex(0, MeaningId);
        M.GetColumnValueByIndex(1, V.PartOfSpeech);
        M.GetColumnValueByIndex(2, V.DisplayOrder);
        M.GetColumnValueByIndex(3, V.ProviderName);
        M.GetColumnValueByIndex(4, V.ProviderKey);
        MeaningIndex.Add(MeaningId, OutRecord.Meanings.Add(MoveTemp(V)));
    }

    for (const auto& Pair : MeaningIndex)
    {
        FSQLitePreparedStatement D;
        if (!D.Create(Database,
                      TEXT("SELECT DefinitionText, UsageExample, DisplayOrder, ProviderName, ProviderKey FROM "
                           "VocabularyDefinitions WHERE MeaningId = ? ORDER BY DisplayOrder, Id;"),
                      ESQLitePreparedStatementFlags::Persistent) ||
            !D.SetBindingValueByIndex(1, Pair.Key))
            return false;
        while (D.Step() == ESQLitePreparedStatementStepResult::Row)
        {
            FEVVocabularyDefinition V;
            D.GetColumnValueByIndex(0, V.DefinitionText);
            D.GetColumnValueByIndex(1, V.UsageExample);
            D.GetColumnValueByIndex(2, V.DisplayOrder);
            D.GetColumnValueByIndex(3, V.ProviderName);
            D.GetColumnValueByIndex(4, V.ProviderKey);
            OutRecord.Meanings[Pair.Value].Definitions.Add(MoveTemp(V));
        }
    }

    FSQLitePreparedStatement T;
    if (!T.Create(Database,
                  TEXT("SELECT COALESCE(MeaningId, 0), TranslationText, TargetLanguage, TargetPartOfSpeech, "
                       "DisplayOrder, ProviderName, ProviderKey, Confidence FROM VocabularyTranslations WHERE EntryId "
                       "= ? ORDER BY DisplayOrder, Id;"),
                  ESQLitePreparedStatementFlags::Persistent) ||
        !T.SetBindingValueByIndex(1, EntryId))
        return false;
    while (T.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        int64 MeaningId = 0;
        FEVVocabularyTranslation V;
        T.GetColumnValueByIndex(0, MeaningId);
        T.GetColumnValueByIndex(1, V.TranslationText);
        T.GetColumnValueByIndex(2, V.TargetLanguage);
        T.GetColumnValueByIndex(3, V.TargetPartOfSpeech);
        T.GetColumnValueByIndex(4, V.DisplayOrder);
        T.GetColumnValueByIndex(5, V.ProviderName);
        T.GetColumnValueByIndex(6, V.ProviderKey);
        double Confidence = 0.0;
        T.GetColumnValueByIndex(7, Confidence);
        V.Confidence = static_cast<float>(Confidence);
        if (MeaningId != 0 && MeaningIndex.Contains(MeaningId))
            OutRecord.Meanings[MeaningIndex[MeaningId]].Translations.Add(MoveTemp(V));
        else
            OutRecord.GeneralTranslations.Add(MoveTemp(V));
    }

    FSQLitePreparedStatement R;
    if (!R.Create(
            Database,
            TEXT("SELECT COALESCE(MeaningId, 0), RelatedWord, NormalizedRelatedWord, RelationType, DisplayOrder, "
                 "ProviderName, ProviderKey FROM VocabularyRelations WHERE EntryId = ? ORDER BY DisplayOrder, Id;"),
            ESQLitePreparedStatementFlags::Persistent) ||
        !R.SetBindingValueByIndex(1, EntryId))
        return false;
    while (R.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        int64 MeaningId = 0;
        FEVVocabularyRelation V;
        R.GetColumnValueByIndex(0, MeaningId);
        R.GetColumnValueByIndex(1, V.RelatedWord);
        R.GetColumnValueByIndex(2, V.NormalizedRelatedWord);
        R.GetColumnValueByIndex(3, V.RelationType);
        R.GetColumnValueByIndex(4, V.DisplayOrder);
        R.GetColumnValueByIndex(5, V.ProviderName);
        R.GetColumnValueByIndex(6, V.ProviderKey);
        if (MeaningId != 0 && MeaningIndex.Contains(MeaningId))
            OutRecord.Meanings[MeaningIndex[MeaningId]].Relations.Add(MoveTemp(V));
    }
    return true;
}

bool UEVVocabularyStorageService::GetRandomlySelectedWord(FString& OutWord)
{
    OutWord.Reset();

    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot retrieve random word: database is invalid."));
        return false;
    }

    FSQLitePreparedStatement Statement;

    if (!Statement.Create(Database, *FEVVocabularySqlQueries::GetRandomlySelectedWordQuery(),
                          ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create random word SELECT statement."));
        return false;
    }

    if (Statement.Step() != ESQLitePreparedStatementStepResult::Row)
    {
        UE_LOG(LogTemp, Warning, TEXT("Vocabulary is empty."));
        return false;
    }

    if (!Statement.GetColumnValueByIndex(0, OutWord))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to read the random word."));
        return false;
    }

    return true;
}

int32 UEVVocabularyStorageService::GetVocabularyEntryCount()
{
    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot count vocabulary entries: database is invalid"));

        return 0;
    }

    FSQLitePreparedStatement Statement;

    if (!Statement.Create(Database, FEVVocabularySqlQueries::CountVocabularyEntries,
                          ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create vocabulary COUNT statement"));

        return 0;
    }

    if (Statement.Step() != ESQLitePreparedStatementStepResult::Row)
    {
        UE_LOG(LogTemp, Error, TEXT("Vocabulary COUNT query returned no row"));

        return 0;
    }

    int32 EntryCount = 0;

    if (!Statement.GetColumnValueByIndex(0, EntryCount))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to read vocabulary entry count"));

        return 0;
    }

    return EntryCount;
}

TArray<FVocabularyEntry> UEVVocabularyStorageService::GetVocabularyEntriesPage(int32 Limit, int32 Offset)
{
    TArray<FVocabularyEntry> Entries;

    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot load vocabulary page: database is invalid"));

        return Entries;
    }

    if (Limit <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Vocabulary page limit must be greater than zero"));

        return Entries;
    }

    if (Offset < 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Vocabulary page offset cannot be negative"));

        return Entries;
    }

    FSQLitePreparedStatement Statement;

    if (!Statement.Create(Database, *FEVVocabularySqlQueries::GetSelectVocabularyEntriesPageQuery(),
                          ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create paginated vocabulary SELECT statement"));

        return Entries;
    }

    // SQLite prepared-statement bindings are 1-based.
    Statement.SetBindingValueByIndex(1, Limit);
    Statement.SetBindingValueByIndex(2, Offset);

    Entries.Reserve(Limit);

    while (Statement.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        FVocabularyEntry Entry;

        if (!ReadEntryFields(Statement, Entry))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to read a paginated vocabulary row"));

            Entries.Reset();
            return Entries;
        }

        Entries.Add(MoveTemp(Entry));
    }

    UE_LOG(LogTemp, Log, TEXT("Loaded vocabulary page: Limit=%d | Offset=%d | Returned=%d"), Limit, Offset,
           Entries.Num());

    return Entries;
}

TArray<FVocabularyEntry> UEVVocabularyStorageService::GetVocabularyEntries(int32 EntryNumber)
{
    TArray<FVocabularyEntry> Entries;

    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot read entries: database is invalid"));
        return Entries;
    }

    if (EntryNumber <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("EntryNumber must be greater than 0"));
        return Entries;
    }

    FSQLitePreparedStatement Statement;

    if (!Statement.Create(Database, *FEVVocabularySqlQueries::GetSelectVocabularyEntriesQuery(),
                          ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create SELECT statement"));
        return Entries;
    }

    while (Statement.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        FVocabularyEntry Entry;

        if (!ReadEntryFields(Statement, Entry))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to read vocabulary entry"));
            Entries.Reset();
            return Entries;
        }

        UE_LOG(LogTemp, Warning, TEXT("Loaded word: %s"), *Entry.Word);

        Entries.Add(Entry);
    }

    UE_LOG(LogTemp, Warning, TEXT("Loaded entries: %d"), Entries.Num());

    return Entries;
}

EEVWordLookupResult UEVVocabularyStorageService::DoesWordExist(const FString& Word, FText& OutErrorMessage)
{
    OutErrorMessage = FText::GetEmpty();

    if (!Database.IsValid())
    {
        OutErrorMessage = FText::FromString(TEXT("The Database is invalid."));
        return EEVWordLookupResult::DatabaseError;
    }

    FSQLitePreparedStatement Statement;

    if (!Statement.Create(Database, FEVVocabularySqlQueries::WordExists, ESQLitePreparedStatementFlags::Persistent))
    {
        OutErrorMessage = FText::FromString(TEXT("Failed to check word existence."));
        return EEVWordLookupResult::DatabaseError;
    }

    Statement.SetBindingValueByIndex(1, FEVWordInputValidator::NormalizeWordInput(Word));

    if (Statement.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        OutErrorMessage = FText::FromString(TEXT("The word you search already exists in the Database"));
        return EEVWordLookupResult::Exists;
    }

    return EEVWordLookupResult::Empty;
}

void UEVVocabularyStorageService::ShutdownStorage()
{
    if (Database.IsValid())
    {
        Database.Close();
        UE_LOG(LogTemp, Warning, TEXT("Database closed"));
    }
}

FEVFileExchangeResultInfo
UEVVocabularyStorageService::GenerateDatabaseExportTemplate(EEVFileExtensionType FileExtensionType,
                                                            TArray<uint8>& OutBytes)
{
    FEVFileExchangeResultInfo ResultInfo;

    OutBytes.Reset();

    switch (FileExtensionType)
    {
    case EEVFileExtensionType::Csv:
    {
        const TArray<FEVDatabaseColumnDefinition> Columns = FEVVocabularyDatabaseSchema::GetImportExportColumns();

        TArray<FString> HeaderFields;
        TArray<FString> HintFields;

        HeaderFields.Reserve(Columns.Num());
        HintFields.Reserve(Columns.Num());

        for (const FEVDatabaseColumnDefinition& Column : Columns)
        {
            HeaderFields.Add(Column.Name);
            HintFields.Add(QuoteCsvField(Column.ImportGuidance));
        }

        FString CsvTemplate;

        CsvTemplate += FString::Join(HeaderFields, TEXT(","));
        CsvTemplate += TEXT("\r\n");

        CsvTemplate += FString::Join(HintFields, TEXT(","));
        CsvTemplate += TEXT("\r\n");

        const FTCHARToUTF8 Utf8Converter(*CsvTemplate);

        OutBytes.Append(reinterpret_cast<const uint8*>(Utf8Converter.Get()), Utf8Converter.Length());

        ResultInfo.Result = EEVFileExchangeResult::Success;
        ResultInfo.ByteSize = OutBytes.Num();
        ResultInfo.UserMessage = TEXT("Database template generated successfully.");
        ResultInfo.DebugMessage = TEXT("CSV template generated dynamically from the database schema.");

        return ResultInfo;
    }

    default:
        ResultInfo.Result = EEVFileExchangeResult::InvalidExtension;
        ResultInfo.UserMessage = TEXT("Unsupported export template format.");
        ResultInfo.DebugMessage = TEXT("GenerateDatabaseExportTemplate received unsupported file extension.");

        return ResultInfo;
    }
}

bool UEVVocabularyStorageService::GetImportExportRows(TArray<FString>& OutColumnNames,
                                                      TArray<FEVDatabaseExportRow>& OutRows)
{
    OutColumnNames = FEVVocabularyDatabaseSchema::GetImportExportColumnNames();
    OutRows.Reset();

    if (!Database.IsValid() || OutColumnNames.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot build export rows: database or exchange schema is unavailable."));
        return false;
    }

    FSQLitePreparedStatement WordStatement;
    if (!WordStatement.Create(Database,
                              TEXT("SELECT NormalizedWord FROM VocabularyEntries ORDER BY Word COLLATE NOCASE ASC;"),
                              ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create normalized export word query."));
        return false;
    }

    while (WordStatement.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        FString NormalizedWord;
        if (!WordStatement.GetColumnValueByIndex(0, NormalizedWord))
            return false;

        FEVVocabularyRecord Record;
        if (!GetVocabularyRecordByWord(NormalizedWord, Record))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load complete export record: %s"), *NormalizedWord);
            return false;
        }

        const FEVVocabularyPronunciation* PrimaryPronunciation = nullptr;
        for (const FEVVocabularyPronunciation& Pronunciation : Record.Pronunciations)
        {
            if (Pronunciation.LanguageCode.Equals(TEXT("en"), ESearchCase::IgnoreCase) && Pronunciation.bPrimary)
            {
                PrimaryPronunciation = &Pronunciation;
                break;
            }
        }
        if (!PrimaryPronunciation)
        {
            PrimaryPronunciation = Record.Pronunciations.FindByPredicate(
                [](const FEVVocabularyPronunciation& Pronunciation)
                { return Pronunciation.LanguageCode.Equals(TEXT("en"), ESearchCase::IgnoreCase); });
        }

        bool bWordLevelFieldsWritten = false;
        auto MakeBaseRow = [&]()
        {
            FEVDatabaseExportRow Row;
            Row.Values.SetNum(OutColumnNames.Num());
            Row.Values[0] = Record.Word;
            if (!bWordLevelFieldsWritten && PrimaryPronunciation)
            {
                Row.Values[2] = PrimaryPronunciation->Transcription;
                Row.Values[3] = PrimaryPronunciation->AudioUrl;
                bWordLevelFieldsWritten = true;
            }
            return Row;
        };

        if (Record.Meanings.IsEmpty())
        {
            FEVDatabaseExportRow Row = MakeBaseRow();
            Row.Values[6] = JoinTranslationValues(Record.GeneralTranslations, TEXT("ru"));
            Row.Values[7] = JoinTranslationValues(Record.GeneralTranslations, TEXT("uk"));
            OutRows.Add(MoveTemp(Row));
            continue;
        }

        for (const FEVVocabularyMeaning& Meaning : Record.Meanings)
        {
            const int32 DefinitionRowCount = FMath::Max(1, Meaning.Definitions.Num());
            for (int32 DefinitionIndex = 0; DefinitionIndex < DefinitionRowCount; ++DefinitionIndex)
            {
                FEVDatabaseExportRow Row = MakeBaseRow();
                Row.Values[1] = Meaning.PartOfSpeech;

                if (Meaning.Definitions.IsValidIndex(DefinitionIndex))
                {
                    Row.Values[4] = Meaning.Definitions[DefinitionIndex].DefinitionText;
                    Row.Values[5] = Meaning.Definitions[DefinitionIndex].UsageExample;
                }

                if (DefinitionIndex == 0)
                {
                    Row.Values[6] = JoinTranslationValues(Meaning.Translations, TEXT("ru"));
                    Row.Values[7] = JoinTranslationValues(Meaning.Translations, TEXT("uk"));
                    Row.Values[8] = JoinRelationValues(Meaning.Relations, TEXT("synonym"));
                    Row.Values[9] = JoinRelationValues(Meaning.Relations, TEXT("antonym"));
                }

                OutRows.Add(MoveTemp(Row));
            }
        }

        if (!Record.GeneralTranslations.IsEmpty())
        {
            FEVDatabaseExportRow GeneralRow = MakeBaseRow();
            GeneralRow.Values[6] = JoinTranslationValues(Record.GeneralTranslations, TEXT("ru"));
            GeneralRow.Values[7] = JoinTranslationValues(Record.GeneralTranslations, TEXT("uk"));
            if (!GeneralRow.Values[6].IsEmpty() || !GeneralRow.Values[7].IsEmpty())
                OutRows.Add(MoveTemp(GeneralRow));
        }
    }

    return true;
}

FEVFileExchangeResultInfo UEVVocabularyStorageService::GenerateDatabaseExport(EEVFileExtensionType FileExtensionType,
                                                                              TArray<uint8>& OutBytes)
{
    FEVFileExchangeResultInfo ResultInfo;
    OutBytes.Reset();

    if (FileExtensionType != EEVFileExtensionType::Csv)
    {
        ResultInfo.Result = EEVFileExchangeResult::InvalidExtension;
        ResultInfo.UserMessage = TEXT("Unsupported database export format.");
        ResultInfo.DebugMessage = TEXT("GenerateDatabaseExport received an unsupported file extension.");
        return ResultInfo;
    }

    TArray<FString> ColumnNames;
    TArray<FEVDatabaseExportRow> Rows;
    if (!GetImportExportRows(ColumnNames, Rows))
    {
        ResultInfo.Result = EEVFileExchangeResult::DatabaseQueryFailed;
        ResultInfo.UserMessage = TEXT("Failed to read database data for export.");
        ResultInfo.DebugMessage = TEXT("GetImportExportRows returned false.");
        return ResultInfo;
    }

    FString CsvContent = FString::Join(ColumnNames, TEXT(",")) + TEXT("\r\n");
    for (const FEVDatabaseExportRow& Row : Rows)
    {
        TArray<FString> EscapedValues;
        EscapedValues.Reserve(Row.Values.Num());
        for (const FString& Value : Row.Values)
            EscapedValues.Add(QuoteCsvField(Value));
        CsvContent += FString::Join(EscapedValues, TEXT(",")) + TEXT("\r\n");
    }

    const FTCHARToUTF8 Utf8Converter(*CsvContent);
    OutBytes.Append(reinterpret_cast<const uint8*>(Utf8Converter.Get()), Utf8Converter.Length());

    ResultInfo.Result = EEVFileExchangeResult::Success;
    ResultInfo.ByteSize = OutBytes.Num();
    ResultInfo.UserMessage = TEXT("Database export generated successfully.");
    ResultInfo.DebugMessage = FString::Printf(TEXT("Flat normalized CSV export generated with %d rows."), Rows.Num());
    return ResultInfo;
}

FEVFileExchangeResultInfo UEVVocabularyStorageService::ValidateImportFile(
    EEVFileExtensionType FileExtensionType, EEVFileOperationType OperationType, const TArray<uint8>& Bytes,
    TArray<uint8>& OutValidationReportBytes, TArray<FEVVocabularyRecord>& OutValidatedRecords)
{
    FEVFileExchangeResultInfo ResultInfo;
    ResultInfo.ByteSize = Bytes.Num();
    OutValidationReportBytes.Reset();
    OutValidatedRecords.Reset();

    if (FileExtensionType != EEVFileExtensionType::Csv)
    {
        ResultInfo.Result = EEVFileExchangeResult::InvalidExtension;
        ResultInfo.UserMessage = TEXT("The selected import format is not supported.");
        ResultInfo.DebugMessage = TEXT("ValidateImportFile received a non-CSV extension.");
        return ResultInfo;
    }
    if (Bytes.IsEmpty())
    {
        ResultInfo.Result = EEVFileExchangeResult::EmptyFile;
        ResultInfo.UserMessage = TEXT("The selected import file is empty.");
        ResultInfo.DebugMessage = TEXT("ValidateImportFile received an empty byte buffer.");
        return ResultInfo;
    }

    const FUTF8ToTCHAR Utf8Converter(reinterpret_cast<const ANSICHAR*>(Bytes.GetData()), Bytes.Num());
    FString CsvText(Utf8Converter.Length(), Utf8Converter.Get());
    CsvText.RemoveFromStart(TEXT("\uFEFF"));
    FCsvParser CsvParser(MoveTemp(CsvText));
    const FCsvParser::FRows& ParsedRows = CsvParser.GetRows();

    if (ParsedRows.Num() < 2)
    {
        ResultInfo.Result = EEVFileExchangeResult::EmptyFile;
        ResultInfo.UserMessage = TEXT("The selected import file contains no vocabulary entries.");
        ResultInfo.DebugMessage = TEXT("CSV must contain a header and at least one data row.");
        return ResultInfo;
    }

    const TArray<const TCHAR*>& HeaderRow = ParsedRows[0];
    TMap<FString, int32> ColumnIndexes;
    TArray<FEVValidationFailedEntry> HeaderProblems;

    for (int32 Index = 0; Index < HeaderRow.Num(); ++Index)
    {
        const FString RawHeader = FString(HeaderRow[Index]).TrimStartAndEnd();
        const FString CanonicalName = CanonicalExchangeColumnName(RawHeader);
        if (CanonicalName.IsEmpty())
        {
            FEVValidationFailedEntry Problem;
            Problem.Scope = EEVValidationProblemScope::Header;
            Problem.RowNumber = 1;
            Problem.Entry = RawHeader.IsEmpty() ? TEXT("<empty column name>") : RawHeader;
            Problem.Reason = FString::Printf(TEXT("Unsupported column name at position %d."), Index + 1);
            HeaderProblems.Add(MoveTemp(Problem));
            continue;
        }
        if (ColumnIndexes.Contains(CanonicalName))
        {
            FEVValidationFailedEntry Problem;
            Problem.Scope = EEVValidationProblemScope::Header;
            Problem.RowNumber = 1;
            Problem.ColumnName = CanonicalName;
            Problem.Entry = RawHeader;
            Problem.Reason = TEXT("The same supported column is declared more than once.");
            HeaderProblems.Add(MoveTemp(Problem));
            continue;
        }
        ColumnIndexes.Add(CanonicalName, Index);
    }

    if (!ColumnIndexes.Contains(TEXT("Word")))
    {
        FEVValidationFailedEntry Problem;
        Problem.Scope = EEVValidationProblemScope::Header;
        Problem.RowNumber = 1;
        Problem.ColumnName = TEXT("Word");
        Problem.Entry = TEXT("<missing>");
        Problem.Reason = TEXT("The Word column is required. Every other supported column is optional.");
        HeaderProblems.Add(MoveTemp(Problem));
    }

    if (!HeaderProblems.IsEmpty())
    {
        const FEVFileExchangeResultInfo ReportResult =
            GenerateValidationReport(FileExtensionType, HeaderProblems, OutValidationReportBytes);
        if (!ReportResult.IsSuccess())
        {
            ResultInfo = ReportResult;
            ResultInfo.UserMessage =
                TEXT("Header problems were found, but the validation report could not be generated.");
            return ResultInfo;
        }
        ResultInfo.Result = EEVFileExchangeResult::InvalidStructure;
        ResultInfo.UserMessage =
            TEXT("The selected CSV contains header problems. Review the generated validation report.");
        ResultInfo.DebugMessage = FString::Printf(TEXT("Header validation found %d problem(s)."), HeaderProblems.Num());
        return ResultInfo;
    }

    const bool bPartOfSpeechColumnPresent = ColumnIndexes.Contains(TEXT("PartOfSpeech"));
    auto GetCell = [&](const TArray<const TCHAR*>& Row, const FString& ColumnName) -> FString
    {
        const int32* Index = ColumnIndexes.Find(ColumnName);
        return Index && Row.IsValidIndex(*Index) ? FString(Row[*Index]).TrimStartAndEnd() : FString();
    };

    TArray<FEVValidationFailedEntry> RowProblems;
    TMap<FString, int32> RecordIndexByWord;
    TMap<FString, TArray<int32>> SourceRowsByNormalizedWord;

    for (int32 RowIndex = 1; RowIndex < ParsedRows.Num(); ++RowIndex)
    {
        const TArray<const TCHAR*>& Row = ParsedRows[RowIndex];
        bool bEntireRowEmpty = true;
        for (const TCHAR* Cell : Row)
        {
            if (!FString(Cell).TrimStartAndEnd().IsEmpty())
            {
                bEntireRowEmpty = false;
                break;
            }
        }
        if (bEntireRowEmpty)
            continue;

        const int32 CsvRowNumber = RowIndex + 1;

        if (Row.Num() > HeaderRow.Num())
        {
            FEVValidationFailedEntry Problem;
            Problem.Scope = EEVValidationProblemScope::Row;
            Problem.RowNumber = CsvRowNumber;
            Problem.Entry = FString::Printf(TEXT("%d columns"), Row.Num());
            Problem.Reason = FString::Printf(TEXT("The row contains %d values, but the header declares %d columns."),
                                             Row.Num(), HeaderRow.Num());
            RowProblems.Add(MoveTemp(Problem));
            continue;
        }

        const FString RawWord = GetCell(Row, TEXT("Word"));

        // The guidance row shipped in the downloadable template is not data.
        if (RawWord.StartsWith(TEXT("WORD - can't be empty"), ESearchCase::IgnoreCase))
            continue;

        FString NormalizedWord;
        FText WordValidationError;
        if (FEVWordInputValidator::ValidateSearchInput(RawWord, NormalizedWord, WordValidationError) !=
            EEVInputValidationResult::Valid)
        {
            FEVValidationFailedEntry Problem;
            Problem.Scope = EEVValidationProblemScope::Row;
            Problem.RowNumber = CsvRowNumber;
            Problem.ColumnName = TEXT("Word");
            Problem.Entry = RawWord;
            Problem.Reason = WordValidationError.ToString();
            RowProblems.Add(MoveTemp(Problem));
            continue;
        }

        SourceRowsByNormalizedWord.FindOrAdd(NormalizedWord).AddUnique(CsvRowNumber);

        int32 RecordIndex = INDEX_NONE;
        if (const int32* ExistingIndex = RecordIndexByWord.Find(NormalizedWord))
        {
            RecordIndex = *ExistingIndex;
        }
        else
        {
            FEVVocabularyRecord Record;
            Record.Word = RawWord;
            Record.NormalizedWord = NormalizedWord;
            RecordIndex = OutValidatedRecords.Add(MoveTemp(Record));
            RecordIndexByWord.Add(NormalizedWord, RecordIndex);
        }
        FEVVocabularyRecord& Record = OutValidatedRecords[RecordIndex];

        const FString PartOfSpeech = GetCell(Row, TEXT("PartOfSpeech"));
        const FString Transcription = GetCell(Row, TEXT("Transcription"));
        const FString AudioUrl = GetCell(Row, TEXT("AudioUrl"));
        const FString DefinitionCell = GetCell(Row, TEXT("Definition"));
        const FString UsageCell = GetCell(Row, TEXT("Usage"));
        const TArray<FString> RuValues = SplitExchangeValues(GetCell(Row, TEXT("TranslationRu")));
        const TArray<FString> UkValues = SplitExchangeValues(GetCell(Row, TEXT("TranslationUa")));
        const TArray<FString> SynonymValues = SplitExchangeValues(GetCell(Row, TEXT("Synonyms")));
        const TArray<FString> AntonymValues = SplitExchangeValues(GetCell(Row, TEXT("Antonyms")));

        if (!Transcription.IsEmpty() || !AudioUrl.IsEmpty())
        {
            const bool bExists = Record.Pronunciations.ContainsByPredicate(
                [&](const FEVVocabularyPronunciation& Existing)
                {
                    return Existing.Transcription.Equals(Transcription, ESearchCase::CaseSensitive) &&
                           Existing.AudioUrl.Equals(AudioUrl, ESearchCase::CaseSensitive);
                });
            if (!bExists)
            {
                FEVVocabularyPronunciation Pronunciation;
                Pronunciation.LanguageCode = TEXT("en");
                Pronunciation.Transcription = Transcription;
                Pronunciation.AudioUrl = AudioUrl;
                Pronunciation.bPrimary = Record.Pronunciations.IsEmpty();
                Pronunciation.DisplayOrder = Record.Pronunciations.Num();
                Record.Pronunciations.Add(MoveTemp(Pronunciation));
            }
        }

        const bool bHasMeaningData = !PartOfSpeech.IsEmpty() || !DefinitionCell.IsEmpty() || !UsageCell.IsEmpty() ||
                                     !SynonymValues.IsEmpty() || !AntonymValues.IsEmpty();
        const bool bGeneralTranslationOnly = bPartOfSpeechColumnPresent && PartOfSpeech.IsEmpty() &&
                                             (!RuValues.IsEmpty() || !UkValues.IsEmpty()) && !bHasMeaningData;

        if (bGeneralTranslationOnly)
        {
            for (const FString& Value : RuValues)
                AddUniqueTranslation(Record.GeneralTranslations, Value, TEXT("ru"), FString());
            for (const FString& Value : UkValues)
                AddUniqueTranslation(Record.GeneralTranslations, Value, TEXT("uk"), FString());
            continue;
        }

        if (bHasMeaningData || !RuValues.IsEmpty() || !UkValues.IsEmpty())
        {
            FEVVocabularyMeaning& Meaning = FindOrAddExchangeMeaning(Record, PartOfSpeech);
            const TArray<FString> DefinitionValues = SplitDefinitionValues(DefinitionCell);
            const TMap<int32, FString> UsageValues = ParseUsageValues(UsageCell);

            if (!DefinitionValues.IsEmpty())
            {
                for (int32 DefinitionIndex = 0; DefinitionIndex < DefinitionValues.Num(); ++DefinitionIndex)
                {
                    const FString& DefinitionText = DefinitionValues[DefinitionIndex];
                    const FString Usage = UsageValues.FindRef(DefinitionIndex);
                    const bool bExists = Meaning.Definitions.ContainsByPredicate(
                        [&](const FEVVocabularyDefinition& Existing)
                        {
                            return Existing.DefinitionText.Equals(DefinitionText, ESearchCase::IgnoreCase) &&
                                   Existing.UsageExample.Equals(Usage, ESearchCase::CaseSensitive);
                        });
                    if (!bExists)
                    {
                        FEVVocabularyDefinition Definition;
                        Definition.DefinitionText = DefinitionText;
                        Definition.UsageExample = Usage;
                        Definition.DisplayOrder = Meaning.Definitions.Num();
                        Meaning.Definitions.Add(MoveTemp(Definition));
                    }
                }
            }
            else if (!UsageCell.IsEmpty())
            {
                for (const TPair<int32, FString>& Pair : UsageValues)
                {
                    FEVVocabularyDefinition Definition;
                    Definition.UsageExample = Pair.Value;
                    Definition.DisplayOrder = Meaning.Definitions.Num();
                    Meaning.Definitions.Add(MoveTemp(Definition));
                }
            }

            for (const FString& Value : RuValues)
                AddUniqueTranslation(Meaning.Translations, Value, TEXT("ru"), Meaning.PartOfSpeech);
            for (const FString& Value : UkValues)
                AddUniqueTranslation(Meaning.Translations, Value, TEXT("uk"), Meaning.PartOfSpeech);
            for (const FString& Value : SynonymValues)
                AddUniqueRelation(Meaning.Relations, Value, TEXT("synonym"));
            for (const FString& Value : AntonymValues)
                AddUniqueRelation(Meaning.Relations, Value, TEXT("antonym"));
        }
    }

    if (!RowProblems.IsEmpty())
    {
        const FEVFileExchangeResultInfo ReportResult =
            GenerateValidationReport(FileExtensionType, RowProblems, OutValidationReportBytes);
        if (!ReportResult.IsSuccess())
        {
            OutValidatedRecords.Reset();
            ResultInfo = ReportResult;
            ResultInfo.UserMessage =
                TEXT("Import-row problems were found, but the validation report could not be generated.");
            return ResultInfo;
        }
        OutValidatedRecords.Reset();
        ResultInfo.Result = EEVFileExchangeResult::InvalidStructure;
        ResultInfo.UserMessage =
            TEXT("The selected CSV contains invalid vocabulary rows. Review the generated validation report.");
        ResultInfo.DebugMessage = FString::Printf(TEXT("Row validation found %d problem(s)."), RowProblems.Num());
        return ResultInfo;
    }

    if (OutValidatedRecords.IsEmpty())
    {
        ResultInfo.Result = EEVFileExchangeResult::EmptyFile;
        ResultInfo.UserMessage = TEXT("The selected import file contains no vocabulary entries.");
        ResultInfo.DebugMessage = TEXT("No valid records remained after skipping empty and template-guidance rows.");
        return ResultInfo;
    }

    if (OperationType == EEVFileOperationType::ImportDBAppend)
    {
        TArray<FEVValidationFailedEntry> DatabaseProblems;
        CollectAppendValidationProblems(OutValidatedRecords, SourceRowsByNormalizedWord, DatabaseProblems);
        if (!DatabaseProblems.IsEmpty())
        {
            const FEVFileExchangeResultInfo ReportResult =
                GenerateValidationReport(FileExtensionType, DatabaseProblems, OutValidationReportBytes);
            if (!ReportResult.IsSuccess())
            {
                OutValidatedRecords.Reset();
                ResultInfo = ReportResult;
                ResultInfo.UserMessage =
                    TEXT("Database conflicts were found, but the validation report could not be generated.");
                return ResultInfo;
            }
            OutValidatedRecords.Reset();
            ResultInfo.Result = EEVFileExchangeResult::InvalidStructure;
            ResultInfo.UserMessage = TEXT("The selected CSV contains words that already exist in the database. Review "
                                          "the generated validation report.");
            ResultInfo.DebugMessage =
                FString::Printf(TEXT("Append validation found %d conflict(s)."), DatabaseProblems.Num());
            return ResultInfo;
        }
    }

    ResultInfo.Result = EEVFileExchangeResult::Success;
    ResultInfo.UserMessage = TEXT("Import file passed validation.");
    ResultInfo.DebugMessage = FString::Printf(TEXT("Flat CSV was grouped into %d normalized vocabulary record(s)."),
                                              OutValidatedRecords.Num());
    return ResultInfo;
}

FEVFileExchangeResultInfo UEVVocabularyStorageService::OverwriteDatabase(const TArray<FEVVocabularyRecord>& Records)
{
    FEVFileExchangeResultInfo ResultInfo;
    if (!Database.IsValid() || Records.IsEmpty())
    {
        ResultInfo.Result =
            Records.IsEmpty() ? EEVFileExchangeResult::EmptyFile : EEVFileExchangeResult::DatabaseTransactionFailed;
        ResultInfo.UserMessage = Records.IsEmpty() ? TEXT("There are no vocabulary entries to import.")
                                                   : TEXT("The database is unavailable.");
        ResultInfo.DebugMessage = TEXT("OverwriteDatabase received invalid input.");
        return ResultInfo;
    }

    if (!Database.Execute(TEXT("BEGIN IMMEDIATE TRANSACTION;")))
    {
        ResultInfo.Result = EEVFileExchangeResult::DatabaseTransactionFailed;
        ResultInfo.UserMessage = TEXT("The database overwrite could not be started.");
        ResultInfo.DebugMessage = TEXT("Failed to begin overwrite transaction.");
        return ResultInfo;
    }
    auto Rollback = [this]() { Database.Execute(TEXT("ROLLBACK;")); };

    if (!Database.Execute(TEXT("DELETE FROM VocabularyEntries;")))
    {
        Rollback();
        ResultInfo.Result = EEVFileExchangeResult::DatabaseTransactionFailed;
        ResultInfo.UserMessage = TEXT("The current database could not be cleared.");
        ResultInfo.DebugMessage = TEXT("Failed to delete normalized vocabulary roots.");
        return ResultInfo;
    }

    for (const FEVVocabularyRecord& Record : Records)
    {
        if (!SaveVocabularyRecordInternal(Record, false))
        {
            Rollback();
            ResultInfo.Result = EEVFileExchangeResult::DatabaseTransactionFailed;
            ResultInfo.UserMessage = TEXT("The imported entries could not be written to the database.");
            ResultInfo.DebugMessage = FString::Printf(TEXT("Failed to import normalized record '%s'."), *Record.Word);
            return ResultInfo;
        }
    }

    if (!Database.Execute(TEXT("COMMIT;")))
    {
        Rollback();
        ResultInfo.Result = EEVFileExchangeResult::DatabaseTransactionFailed;
        ResultInfo.UserMessage = TEXT("The database overwrite could not be completed.");
        ResultInfo.DebugMessage = TEXT("Failed to commit overwrite transaction.");
        return ResultInfo;
    }

    ResultInfo.Result = EEVFileExchangeResult::Success;
    ResultInfo.UserMessage = TEXT("The database was overwritten successfully.");
    ResultInfo.DebugMessage = FString::Printf(TEXT("Imported %d normalized record(s)."), Records.Num());
    return ResultInfo;
}

FEVFileExchangeResultInfo UEVVocabularyStorageService::AppendDatabase(const TArray<FEVVocabularyRecord>& Records)
{
    FEVFileExchangeResultInfo ResultInfo;
    if (!Database.IsValid() || Records.IsEmpty())
    {
        ResultInfo.Result =
            Records.IsEmpty() ? EEVFileExchangeResult::EmptyFile : EEVFileExchangeResult::DatabaseTransactionFailed;
        ResultInfo.UserMessage = Records.IsEmpty() ? TEXT("There are no vocabulary entries to append.")
                                                   : TEXT("The database is unavailable.");
        ResultInfo.DebugMessage = TEXT("AppendDatabase received invalid input.");
        return ResultInfo;
    }

    if (!Database.Execute(TEXT("BEGIN IMMEDIATE TRANSACTION;")))
    {
        ResultInfo.Result = EEVFileExchangeResult::DatabaseTransactionFailed;
        ResultInfo.UserMessage = TEXT("The database append could not be started.");
        ResultInfo.DebugMessage = TEXT("Failed to begin append transaction.");
        return ResultInfo;
    }
    auto Rollback = [this]() { Database.Execute(TEXT("ROLLBACK;")); };

    for (const FEVVocabularyRecord& Record : Records)
    {
        if (!SaveVocabularyRecordInternal(Record, false))
        {
            Rollback();
            ResultInfo.Result = EEVFileExchangeResult::DatabaseTransactionFailed;
            ResultInfo.UserMessage = TEXT("The imported entries could not be appended to the database.");
            ResultInfo.DebugMessage = FString::Printf(TEXT("Failed to append normalized record '%s'."), *Record.Word);
            return ResultInfo;
        }
    }

    if (!Database.Execute(TEXT("COMMIT;")))
    {
        Rollback();
        ResultInfo.Result = EEVFileExchangeResult::DatabaseTransactionFailed;
        ResultInfo.UserMessage = TEXT("The database append could not be completed.");
        ResultInfo.DebugMessage = TEXT("Failed to commit append transaction.");
        return ResultInfo;
    }

    ResultInfo.Result = EEVFileExchangeResult::Success;
    ResultInfo.UserMessage = TEXT("The imported entries were appended successfully.");
    ResultInfo.DebugMessage = FString::Printf(TEXT("Appended %d normalized record(s)."), Records.Num());
    return ResultInfo;
}

FEVFileExchangeResultInfo
UEVVocabularyStorageService::GenerateValidationReport(EEVFileExtensionType FileExtensionType,
                                                      const TArray<FEVValidationFailedEntry>& InvalidEntries,
                                                      TArray<uint8>& OutBytes)
{
    FEVFileExchangeResultInfo ResultInfo;

    OutBytes.Reset();

    if (FileExtensionType != EEVFileExtensionType::Csv)
    {
        ResultInfo.Result = EEVFileExchangeResult::InvalidExtension;
        ResultInfo.UserMessage = TEXT("The selected report format is not supported.");
        ResultInfo.DebugMessage = TEXT("GenerateValidationReport received a non-CSV extension.");

        return ResultInfo;
    }

    if (InvalidEntries.IsEmpty())
    {
        ResultInfo.Result = EEVFileExchangeResult::EmptyFile;
        ResultInfo.UserMessage = TEXT("There are no problematic entries to report.");
        ResultInfo.DebugMessage = TEXT("GenerateValidationReport received no entries.");

        return ResultInfo;
    }

    const FString CsvContent =
        FEVImportValidationReportFormatter::BuildCsvReport(FEVVocabularyDatabaseSchema::GetImportExportColumns(),
                                                           FEVImportValidationRules::GetGeneralRules(), InvalidEntries);

    const FTCHARToUTF8 Utf8Converter(*CsvContent);

    OutBytes.Append(reinterpret_cast<const uint8*>(Utf8Converter.Get()), Utf8Converter.Length());

    ResultInfo.Result = EEVFileExchangeResult::Success;
    ResultInfo.ByteSize = OutBytes.Num();
    ResultInfo.UserMessage = TEXT("Import problem report generated successfully.");
    ResultInfo.DebugMessage =
        FString::Printf(TEXT("Generated CSV report containing %d problematic entries."), InvalidEntries.Num());

    return ResultInfo;
}

int32 UEVVocabularyStorageService::GetVocabularyEntryCountByPrefix(const FString& SearchPrefix)
{
    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot count filtered vocabulary entries: database is invalid"));

        return 0;
    }

    FSQLitePreparedStatement Statement;

    if (!Statement.Create(Database, FEVVocabularySqlQueries::GetVocabularyEntryCountByPrefixQuery,
                          ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create filtered vocabulary COUNT statement"));

        return 0;
    }

    const FString SearchPattern = FEVWordInputValidator::NormalizeWordInput(SearchPrefix) + TEXT("%");

    if (!Statement.SetBindingValueByIndex(1, SearchPattern))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to bind vocabulary search pattern: %s"), *SearchPattern);

        return 0;
    }

    if (Statement.Step() != ESQLitePreparedStatementStepResult::Row)
    {
        UE_LOG(LogTemp, Error, TEXT("Filtered vocabulary COUNT query returned no row"));

        return 0;
    }

    int32 EntryCount = 0;

    if (!Statement.GetColumnValueByIndex(0, EntryCount))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to read filtered vocabulary entry count"));

        return 0;
    }

    return EntryCount;
}

TArray<FVocabularyEntry> UEVVocabularyStorageService::GetVocabularyEntriesPageByPrefix(const FString& SearchPrefix,
                                                                                       int32 Limit, int32 Offset)
{
    TArray<FVocabularyEntry> Entries;

    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot load filtered vocabulary page: database is invalid"));

        return Entries;
    }

    if (Limit <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Filtered vocabulary page limit must be greater than zero"));

        return Entries;
    }

    if (Offset < 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Filtered vocabulary page offset cannot be negative"));

        return Entries;
    }

    FSQLitePreparedStatement Statement;

    if (!Statement.Create(Database, *FEVVocabularySqlQueries::GetSelectVocabularyEntriesPageByPrefixQuery(),
                          ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create filtered paginated vocabulary SELECT statement"));

        return Entries;
    }

    const FString SearchPattern = FEVWordInputValidator::NormalizeWordInput(SearchPrefix) + TEXT("%");

    if (!Statement.SetBindingValueByIndex(1, SearchPattern) || !Statement.SetBindingValueByIndex(2, Limit) ||
        !Statement.SetBindingValueByIndex(3, Offset))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to bind filtered vocabulary page parameters"));

        return Entries;
    }

    Entries.Reserve(Limit);

    while (Statement.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        FVocabularyEntry Entry;

        if (!ReadEntryFields(Statement, Entry))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to read a filtered vocabulary row"));

            Entries.Reset();
            return Entries;
        }

        Entries.Add(MoveTemp(Entry));
    }

    UE_LOG(LogTemp, Log, TEXT("Loaded filtered vocabulary page: Prefix=%s | Limit=%d | Offset=%d | Returned=%d"),
           *SearchPrefix, Limit, Offset, Entries.Num());

    return Entries;
}

bool UEVVocabularyStorageService::InsertVocabularyEntryStrict(const FVocabularyEntry& Entry)
{
    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot strictly insert entry: database is invalid."));
        return false;
    }

    // Import append/overwrite already owns the surrounding transaction.
    return SaveVocabularyRecordInternal(ConvertLegacyEntryToRecord(Entry), false);
}

void UEVVocabularyStorageService::CollectAppendValidationProblems(
    const TArray<FEVVocabularyRecord>& Records, const TMap<FString, TArray<int32>>& SourceRowsByNormalizedWord,
    TArray<FEVValidationFailedEntry>& OutProblems)
{
    OutProblems.Reset();

    for (const FEVVocabularyRecord& Record : Records)
    {
        const FString NormalizedWord = Record.NormalizedWord.IsEmpty()
                                           ? FEVWordInputValidator::NormalizeWordInput(Record.Word)
                                           : Record.NormalizedWord;

        FText ErrorMessage;
        if (DoesWordExist(NormalizedWord, ErrorMessage) != EEVWordLookupResult::Exists)
        {
            continue;
        }

        const TArray<int32>* SourceRows = SourceRowsByNormalizedWord.Find(NormalizedWord);
        if (!SourceRows || SourceRows->IsEmpty())
        {
            FEVValidationFailedEntry Problem;
            Problem.Scope = EEVValidationProblemScope::Database;
            Problem.ColumnName = TEXT("Word");
            Problem.Entry = Record.Word;
            Problem.Reason = TEXT("The word already exists in the current database and cannot be appended.");
            OutProblems.Add(MoveTemp(Problem));
            continue;
        }

        for (const int32 SourceRowNumber : *SourceRows)
        {
            FEVValidationFailedEntry Problem;
            Problem.Scope = EEVValidationProblemScope::Database;
            Problem.RowNumber = SourceRowNumber;
            Problem.ColumnName = TEXT("Word");
            Problem.Entry = Record.Word;
            Problem.Reason = TEXT("The word already exists in the current database and cannot be appended.");
            OutProblems.Add(MoveTemp(Problem));
        }
    }
}
int32 UEVVocabularyStorageService::GetVocabularyEntryCountByCriteria(const FString& SearchPrefix,
                                                                     const FEVVocabularyQueryCriteria& Criteria)
{
    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot count vocabulary entries by criteria: database is invalid"));
        return 0;
    }

    const TArray<FString> PartsOfSpeech = EVVocabularyFilter::GetPartOfSpeechDatabaseValues(Criteria);
    const bool bHasPrefix = !SearchPrefix.TrimStartAndEnd().IsEmpty();

    FSQLitePreparedStatement Statement;
    const FString Query =
        FEVVocabularySqlQueries::GetVocabularyEntryCountByCriteriaQuery(PartsOfSpeech.Num(), bHasPrefix);
    if (!Statement.Create(Database, *Query, ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create vocabulary criteria COUNT statement"));
        return 0;
    }

    int32 BindingIndex = 1;
    if (bHasPrefix)
    {
        const FString SearchPattern = FEVWordInputValidator::NormalizeWordInput(SearchPrefix) + TEXT("%");
        if (!Statement.SetBindingValueByIndex(BindingIndex++, SearchPattern))
        {
            return 0;
        }
    }
    for (const FString& PartOfSpeech : PartsOfSpeech)
    {
        if (!Statement.SetBindingValueByIndex(BindingIndex++, PartOfSpeech))
        {
            return 0;
        }
    }

    if (Statement.Step() != ESQLitePreparedStatementStepResult::Row)
    {
        return 0;
    }

    int32 EntryCount = 0;
    return Statement.GetColumnValueByIndex(0, EntryCount) ? EntryCount : 0;
}

TArray<FVocabularyEntry> UEVVocabularyStorageService::GetVocabularyEntriesPageByCriteria(
    const FString& SearchPrefix, const FEVVocabularyQueryCriteria& Criteria, const int32 Limit, const int32 Offset)
{
    TArray<FVocabularyEntry> Entries;
    if (!Database.IsValid() || Limit <= 0 || Offset < 0)
    {
        return Entries;
    }

    const TArray<FString> PartsOfSpeech = EVVocabularyFilter::GetPartOfSpeechDatabaseValues(Criteria);
    const bool bHasPrefix = !SearchPrefix.TrimStartAndEnd().IsEmpty();

    FSQLitePreparedStatement Statement;
    const FString Query =
        FEVVocabularySqlQueries::GetSelectVocabularyEntriesPageByCriteriaQuery(PartsOfSpeech.Num(), bHasPrefix);
    if (!Statement.Create(Database, *Query, ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create vocabulary criteria page statement"));
        return Entries;
    }

    int32 BindingIndex = 1;
    if (bHasPrefix)
    {
        const FString SearchPattern = FEVWordInputValidator::NormalizeWordInput(SearchPrefix) + TEXT("%");
        if (!Statement.SetBindingValueByIndex(BindingIndex++, SearchPattern))
        {
            return Entries;
        }
    }
    for (const FString& PartOfSpeech : PartsOfSpeech)
    {
        if (!Statement.SetBindingValueByIndex(BindingIndex++, PartOfSpeech))
        {
            return Entries;
        }
    }
    if (!Statement.SetBindingValueByIndex(BindingIndex++, Limit) ||
        !Statement.SetBindingValueByIndex(BindingIndex, Offset))
    {
        return Entries;
    }

    Entries.Reserve(Limit);
    while (Statement.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        FVocabularyEntry Entry;
        if (!ReadEntryFields(Statement, Entry))
        {
            Entries.Reset();
            return Entries;
        }
        Entry.NormalizedWord = FEVWordInputValidator::NormalizeWordInput(Entry.Word);
        Entry.bHasUsageExamples = EVVocabularyUsage::HasUsageExamples(Entry.Usage);
        Entries.Add(MoveTemp(Entry));
    }
    return Entries;
}
