// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVVocabularyTypes.h"
#include "EVVocabularyFilterTypes.h"
#include "SQLiteDatabase.h"
#include "EVFileExchangeTypes.h"
#include "EVImportValidationTypes.h"
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

struct FEVDatabaseExportRow
{
    TArray<FString> Values;
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

    bool SaveVocabularyRecord(const FEVVocabularyRecord& Record);
    bool UpdateVocabularyRecord(const FEVVocabularyRecord& Record);
    bool GetVocabularyRecordByWord(const FString& NormalizedWord, FEVVocabularyRecord& OutRecord);
    bool DeleteVocabularyRecord(const FString& NormalizedWord);

    bool GetVocabularyEntryByWord(const FString& Word, FVocabularyEntry& OutEntry);
    bool GetRandomlySelectedWord(FString& OutWord);

    int32 GetVocabularyEntryCount();

    TArray<FVocabularyEntry> GetVocabularyEntriesPage(int32 Limit, int32 Offset);

    TArray<FVocabularyEntry> GetVocabularyEntries(int32 EntryNumber = 5);

    void ShutdownStorage();

    EEVWordLookupResult DoesWordExist(const FString& Word, FText& OutErrorMessage);

    FEVFileExchangeResultInfo GenerateDatabaseExportTemplate(EEVFileExtensionType FileExtensionType,
                                                             TArray<uint8>& OutBytes);

    bool GetImportExportRows(TArray<FString>& OutColumnNames, TArray<FEVDatabaseExportRow>& OutRows);

    FEVFileExchangeResultInfo GenerateDatabaseExport(EEVFileExtensionType FileExtensionType, TArray<uint8>& OutBytes);

    FEVFileExchangeResultInfo ValidateImportFile(EEVFileExtensionType FileExtensionType,
                                                 EEVFileOperationType OperationType, const TArray<uint8>& Bytes,
                                                 TArray<uint8>& OutValidationReportBytes,
                                                 TArray<FEVVocabularyRecord>& OutValidatedRecords);

    FEVFileExchangeResultInfo OverwriteDatabase(const TArray<FEVVocabularyRecord>& Records);

    FEVFileExchangeResultInfo AppendDatabase(const TArray<FEVVocabularyRecord>& Records);

    FEVFileExchangeResultInfo GenerateValidationReport(EEVFileExtensionType FileExtensionType,
                                                       const TArray<FEVValidationFailedEntry>& InvalidEntries,
                                                       TArray<uint8>& OutBytes);

    int32 GetVocabularyEntryCountByPrefix(const FString& SearchPrefix);

    TArray<FVocabularyEntry> GetVocabularyEntriesPageByPrefix(const FString& SearchPrefix, int32 Limit, int32 Offset);

    int32 GetVocabularyEntryCountByCriteria(const FString& SearchPrefix, const FEVVocabularyQueryCriteria& Criteria);
    TArray<FVocabularyEntry> GetVocabularyEntriesPageByCriteria(const FString& SearchPrefix,
                                                                const FEVVocabularyQueryCriteria& Criteria, int32 Limit,
                                                                int32 Offset);

private:
    FSQLiteDatabase Database;

    bool InitializeSchema();
    bool CreateNormalizedSchema();
    bool MigrateLegacySchema();
    bool MigrateNormalizedSchemaToVersion3(int32 CurrentVersion);
    bool IsLegacySchema(bool& bOutIsLegacy);
    bool GetSchemaVersion(int32& OutVersion);
    bool SetSchemaVersion(int32 Version);

    bool SaveVocabularyRecordInternal(const FEVVocabularyRecord& Record, bool bManageTransaction);
    bool ReplaceVocabularyRecordInternal(const FEVVocabularyRecord& Record, bool bManageTransaction);
    bool InsertRecordChildren(int64 EntryId, const FEVVocabularyRecord& Record);
    bool GetEntryIdByNormalizedWord(const FString& NormalizedWord, int64& OutEntryId);
    bool GetLastInsertedRowId(int64& OutRowId);

    static FEVVocabularyRecord ConvertLegacyEntryToRecord(const FVocabularyEntry& Entry);
    static FVocabularyEntry FlattenRecord(const FEVVocabularyRecord& Record);

    bool InsertVocabularyEntryStrict(const FVocabularyEntry& Entry);
    void CollectAppendValidationProblems(const TArray<FEVVocabularyRecord>& Records,
                                         const TMap<FString, TArray<int32>>& SourceRowsByNormalizedWord,
                                         TArray<FEVValidationFailedEntry>& OutProblems);
};
