// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVVocabularyTypes.h"
#include "SQLiteDatabase.h"
#include "EVFileExchangeTypes.h"
#include "EVVocabularyStorageService.generated.h"

/**
 *
 */

UENUM()
enum class EEVWordLookupResult : uint8
{
    Exists,
    DoesNotExist,
    DatabaseError,
    Empty
};

UCLASS()
class ENHANCEVOCABULARYSTORAGE_API UEVVocabularyStorageService : public UObject
{
    GENERATED_BODY()

public:
    bool InitializeStorage();
    bool SaveVocabularyEntry(const FVocabularyEntry& Entry);
    bool UpdateVocabularyEntry(const FVocabularyEntry& Entry);
    bool DeleteVocabularyEntry(const FVocabularyEntry& Entry);

    bool GetVocabularyEntryByWord(const FString& Word, FVocabularyEntry& OutEntry);
    TArray<FVocabularyEntry> GetVocabularyEntries(int32 EntryNumber = 5);
    void ShutdownStorage();
    EEVWordLookupResult DoesWordExist(const FString& Word, FText& OutErrorMessage);

    FEVFileExchangeResultInfo GenerateDatabaseExportTemplate(EEVFileExtensionType FileExtensionType,
                                                             TArray<uint8>& OutBytes);

private:
    FSQLiteDatabase Database;
    bool CreateVocabularyTable();
};
