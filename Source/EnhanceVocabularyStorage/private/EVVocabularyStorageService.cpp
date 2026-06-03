// Fill out your copyright notice in the Description page of Project Settings.

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

    UE_LOG(LogTemp, Warning, TEXT("Database opened successfully"));

    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("The Database is invalid"));
        return false;
    }

    return CreateVocabularyTable();
}

FString UEVVocabularyStorageService::GetDatabaseFilePath() const
{
    return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("EnhanceVocabulary.db"));
}

bool UEVVocabularyStorageService::CreateVocabularyTable()
{
    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot create table: database is invalid"));
        return false;
    }

    const TCHAR* CreateTableSql = TEXT("CREATE TABLE IF NOT EXISTS VocabularyEntries ("
                                       "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                       "Word TEXT NOT NULL,"
                                       "Definition TEXT,"
                                       "Usage TEXT,"
                                       "TranslationRu TEXT,"
                                       "TranslationUa TEXT,"
                                       "CreatedAt TEXT NOT NULL,"
                                       "UpdatedAt TEXT NOT NULL"
                                       ");");

    if (!Database.Execute(CreateTableSql))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create VocabularyEntries table"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("VocabularyEntries table ready"));
    return true;
}

bool UEVVocabularyStorageService::SaveVocabularyEntry(const FVocabularyEntry& Entry)
{
    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot save entry: database is invalid"));
        return false;
    }

    const FString Now = FDateTime::UtcNow().ToIso8601();

    FSQLitePreparedStatement Statement;
    Statement.Create(Database,
                     TEXT("INSERT INTO VocabularyEntries "
                          "(Word, Definition, Usage, TranslationRu, TranslationUa, CreatedAt, UpdatedAt) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?);"),
                     ESQLitePreparedStatementFlags::Persistent);

    Statement.SetBindingValueByIndex(1, Entry.Word);
    Statement.SetBindingValueByIndex(2, Entry.Definition);
    Statement.SetBindingValueByIndex(3, Entry.Usage);
    Statement.SetBindingValueByIndex(4, Entry.TranslationRu);
    Statement.SetBindingValueByIndex(5, Entry.TranslationUa);
    Statement.SetBindingValueByIndex(6, Now);
    Statement.SetBindingValueByIndex(7, Now);

    if (!Statement.Execute())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to insert vocabulary entry: %s"), *Entry.Word);
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("Saved word to DB: %s"), *Entry.Word);
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
        return Entries;
    }

    FSQLitePreparedStatement Statement;
    Statement.Create(Database,
                     TEXT("SELECT Word, Definition, Usage, TranslationRu, TranslationUa "
                          "FROM VocabularyEntries "
                          "ORDER BY Word ASC "
                          "LIMIT ?;"),
                     ESQLitePreparedStatementFlags::Persistent);

    Statement.SetBindingValueByIndex(1, EntryNumber);

    while (Statement.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        FVocabularyEntry Entry;
        Statement.GetColumnValueByIndex(0, Entry.Word);
        Statement.GetColumnValueByIndex(1, Entry.Definition);
        Statement.GetColumnValueByIndex(2, Entry.Usage);
        Statement.GetColumnValueByIndex(3, Entry.TranslationRu);
        Statement.GetColumnValueByIndex(4, Entry.TranslationUa);

        Entries.Add(Entry);
    }

    return Entries;
}

void UEVVocabularyStorageService::ShutdownStorage()
{
    if (Database.IsValid())
    {
        Database.Close();
        UE_LOG(LogTemp, Warning, TEXT("Database closed"));
    }
}
