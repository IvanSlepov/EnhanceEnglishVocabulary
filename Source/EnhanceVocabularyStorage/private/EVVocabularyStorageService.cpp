#include "EVVocabularyStorageService.h"

#include "Misc/Paths.h"
#include "SQLitePreparedStatement.h"

bool UEVVocabularyStorageService::InitializeStorage()
{
    const FString DbPath = GetDatabaseFilePath();

    UE_LOG(LogTemp, Warning, TEXT("DB path: %s"), *DbPath);

    if (!Database.Open(*DbPath, ESQLiteDatabaseOpenMode::ReadWriteCreate))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to open database"));
        return false;
    }

    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Database is invalid"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("Database opened successfully"));

    return CreateVocabularyTable();
}

FString UEVVocabularyStorageService::GetDatabaseFilePath() const
{
    return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("EnhanceVocabulary.db"));
}

bool UEVVocabularyStorageService::CreateVocabularyTable()
{
    const TCHAR* CreateTableSql = TEXT("CREATE TABLE IF NOT EXISTS VocabularyEntries ("
                                       "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                       "Word TEXT NOT NULL,"
                                       "Definition TEXT,"
                                       "Usage TEXT,"
                                       "TranslationRu TEXT,"
                                       "TranslationUa TEXT,"
                                       "CreatedAt TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                                       "UpdatedAt TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"
                                       ");");

    if (!Database.Execute(CreateTableSql))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create VocabularyEntries table"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("VocabularyEntries table ready"));

    LogVocabularyEntryCount();

    return true;
}

bool UEVVocabularyStorageService::SaveVocabularyEntry(const FVocabularyEntry& Entry)
{
    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot save entry: database is invalid"));
        return false;
    }

    FSQLitePreparedStatement Statement;

    if (!Statement.Create(Database,
                          TEXT("INSERT INTO VocabularyEntries "
                               "(Word, Definition, Usage, TranslationRu, TranslationUa) "
                               "VALUES (?, ?, ?, ?, ?);"),
                          ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create INSERT statement"));
        return false;
    }

    // 1-based
    Statement.SetBindingValueByIndex(1, Entry.Word);
    Statement.SetBindingValueByIndex(2, Entry.Definition);
    Statement.SetBindingValueByIndex(3, Entry.Usage);
    Statement.SetBindingValueByIndex(4, Entry.TranslationRu);
    Statement.SetBindingValueByIndex(5, Entry.TranslationUa);

    if (!Statement.Execute())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to insert vocabulary entry: %s"), *Entry.Word);
        return false;
    }

    LogVocabularyEntryCount();

    return true;
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

    LogVocabularyEntryCount();

    FSQLitePreparedStatement Statement;

    if (!Statement.Create(Database,
                          TEXT("SELECT Word, Definition, Usage, TranslationRu, TranslationUa "
                               "FROM VocabularyEntries "
                               "ORDER BY Word ASC;"),
                          ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create SELECT statement"));
        return Entries;
    }

    while (Statement.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        FVocabularyEntry Entry;

        // 0-based
        Statement.GetColumnValueByIndex(0, Entry.Word);
        Statement.GetColumnValueByIndex(1, Entry.Definition);
        Statement.GetColumnValueByIndex(2, Entry.Usage);
        Statement.GetColumnValueByIndex(3, Entry.TranslationRu);
        Statement.GetColumnValueByIndex(4, Entry.TranslationUa);

        UE_LOG(LogTemp, Warning, TEXT("Loaded word: %s"), *Entry.Word);

        Entries.Add(Entry);
    }

    UE_LOG(LogTemp, Warning, TEXT("Loaded entries: %d"), Entries.Num());

    return Entries;
}

void UEVVocabularyStorageService::LogVocabularyEntryCount()
{
    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot count entries: database is invalid"));
        return;
    }

    FSQLitePreparedStatement CountStatement;

    if (!CountStatement.Create(Database, TEXT("SELECT COUNT(*) FROM VocabularyEntries;"),
                               ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create COUNT statement"));
        return;
    }

    if (CountStatement.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        int32 Count = 0;
        CountStatement.GetColumnValueByIndex(0, Count);

        UE_LOG(LogTemp, Warning, TEXT("VocabularyEntries row count: %d"), Count);
    }
}

void UEVVocabularyStorageService::ShutdownStorage()
{
    if (Database.IsValid())
    {
        Database.Close();
        UE_LOG(LogTemp, Warning, TEXT("Database closed"));
    }
}