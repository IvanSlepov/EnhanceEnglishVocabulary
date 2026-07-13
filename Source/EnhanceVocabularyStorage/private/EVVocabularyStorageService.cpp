#include "EVVocabularyStorageService.h"

#include "EVVocabularyDatabaseSchema.h"
#include "EVVocabularySqlQueries.h"
#include "EVWordInputValidator.h"

#include "Misc/Paths.h"
#include "SQLitePreparedStatement.h"
#include "EVHelpers.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Serialization/Csv/CsvParser.h"

bool UEVVocabularyStorageService::InitializeStorage()
{
#if WITH_EDITOR

    const FString DebugDbPath = UEVHelpers::GetVocabularyDebugDbPath();

    UE_LOG(LogTemp, Warning, TEXT("Editor Debug DB path: %s"), *DebugDbPath);

    IFileManager::Get().MakeDirectory(*FPaths::GetPath(DebugDbPath), true);

    if (!Database.Open(*DebugDbPath, ESQLiteDatabaseOpenMode::ReadWriteCreate))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to open/create editor debug database"));
        return false;
    }

#else

    const FString LiveDbPath = UEVHelpers::GetVocabularyLiveDbPath();

    UE_LOG(LogTemp, Warning, TEXT("Packaged Live DB path: %s"), *LiveDbPath);

    IFileManager::Get().MakeDirectory(*FPaths::GetPath(LiveDbPath), true);

    if (!Database.Open(*LiveDbPath, ESQLiteDatabaseOpenMode::ReadWriteCreate))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to open/create packaged live database"));
        return false;
    }

#endif

    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Database is invalid"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("Database opened successfully"));

    return CreateVocabularyTable();
}

bool UEVVocabularyStorageService::CreateVocabularyTable()
{
    const FString CreateTableQuery = FEVVocabularySqlQueries::GetCreateVocabularyTableQuery();

    if (!Database.Execute(*CreateTableQuery))
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

    FSQLitePreparedStatement Statement;

    if (!Statement.Create(Database, FEVVocabularySqlQueries::InsertVocabularyEntry,
                          ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create INSERT statement"));
        return false;
    }

    UE_LOG(LogTemp, Error, TEXT("Word is: %s"), *Entry.Word);

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

    return true;
}

bool UEVVocabularyStorageService::UpdateVocabularyEntry(const FVocabularyEntry& Entry)
{
    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot update entry: database is invalid"));
        return false;
    }

    FSQLitePreparedStatement Statement;

    if (!Statement.Create(Database, FEVVocabularySqlQueries::EditVocabularyEntry,
                          ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create UPDATE statement"));
        return false;
    }

    // 1-based
    Statement.SetBindingValueByIndex(1, Entry.Definition);
    Statement.SetBindingValueByIndex(2, Entry.Usage);
    Statement.SetBindingValueByIndex(3, Entry.TranslationRu);
    Statement.SetBindingValueByIndex(4, Entry.TranslationUa);
    Statement.SetBindingValueByIndex(5, Entry.Word);

    if (!Statement.Execute())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to update vocabulary entry: %s"), *Entry.Word);
        return false;
    }

    return true;
}

bool UEVVocabularyStorageService::DeleteVocabularyEntry(const FVocabularyEntry& Entry)
{
    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot delete entry: database is invalid"));
        return false;
    }

    FSQLitePreparedStatement Statement;

    if (!Statement.Create(Database, FEVVocabularySqlQueries::DeleteVocabularyEntry,
                          ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create DELETE statement"));
        return false;
    }

    // 1-based
    Statement.SetBindingValueByIndex(1, Entry.Word);

    if (!Statement.Execute())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to delete vocabulary entry: %s"), *Entry.Word);
        return false;
    }

    return true;
}

bool UEVVocabularyStorageService::GetVocabularyEntryByWord(const FString& Word, FVocabularyEntry& OutEntry)
{
    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot retrieve entry: database is invalid"));
        return false;
    }

    FSQLitePreparedStatement Statement;

    if (!Statement.Create(Database, FEVVocabularySqlQueries::GetVocabularyEntryByWord,
                          ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create SELECT statement"));
        return false;
    }

    // 1-based
    Statement.SetBindingValueByIndex(1, Word);

    if (!Statement.Execute())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to execute SELECT statement for word: %s"), *Word);
        return false;
    }

    const ESQLitePreparedStatementStepResult StepResult = Statement.Step();

    if (StepResult != ESQLitePreparedStatementStepResult::Row)
    {
        return false;
    }

    Statement.GetColumnValueByIndex(0, OutEntry.Word);
    Statement.GetColumnValueByIndex(1, OutEntry.Definition);
    Statement.GetColumnValueByIndex(2, OutEntry.Usage);
    Statement.GetColumnValueByIndex(3, OutEntry.TranslationRu);
    Statement.GetColumnValueByIndex(4, OutEntry.TranslationUa);

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

    FSQLitePreparedStatement Statement;

    if (!Statement.Create(Database, FEVVocabularySqlQueries::SelectVocabularyEntries,
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

    Statement.SetBindingValueByIndex(1, Word);

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
        const TArray<FString> ColumnNames = FEVVocabularyDatabaseSchema::GetImportExportColumnNames();

        if (ColumnNames.IsEmpty())
        {
            ResultInfo.Result = EEVFileExchangeResult::InvalidStructure;
            ResultInfo.UserMessage = TEXT("No import/export columns are configured.");
            ResultInfo.DebugMessage = TEXT("The vocabulary schema contains no ImportExport columns.");

            return ResultInfo;
        }

        FString CsvTemplate = FString::Join(ColumnNames, TEXT(","));

        CsvTemplate += TEXT("\r\n");

        const FTCHARToUTF8 Utf8Converter(*CsvTemplate);

        OutBytes.Append(reinterpret_cast<const uint8*>(Utf8Converter.Get()), Utf8Converter.Length());

        ResultInfo.Result = EEVFileExchangeResult::Success;
        ResultInfo.ByteSize = OutBytes.Num();
        ResultInfo.UserMessage = TEXT("Database template generated successfully.");
        ResultInfo.DebugMessage = TEXT("CSV template generated from the database schema.");

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
    OutColumnNames.Reset();
    OutRows.Reset();

    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot read import/export rows: database is invalid"));

        return false;
    }

    OutColumnNames = FEVVocabularyDatabaseSchema::GetImportExportColumnNames();

    if (OutColumnNames.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot read import/export rows: no import/export columns are configured"));

        return false;
    }

    const FString SelectQuery = FEVVocabularySqlQueries::GetSelectImportExportColumnsQuery();

    if (SelectQuery.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot read import/export rows: SELECT query is empty"));

        return false;
    }

    FSQLitePreparedStatement Statement;

    if (!Statement.Create(Database, *SelectQuery, ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create import/export SELECT statement"));

        return false;
    }

    while (Statement.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        FEVDatabaseExportRow Row;
        Row.Values.Reserve(OutColumnNames.Num());

        for (int32 ColumnIndex = 0; ColumnIndex < OutColumnNames.Num(); ++ColumnIndex)
        {
            FString Value;

            if (!Statement.GetColumnValueByIndex(ColumnIndex, Value))
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to read import/export column at index %d"), ColumnIndex);

                OutRows.Reset();
                return false;
            }

            Row.Values.Add(MoveTemp(Value));
        }

        OutRows.Add(MoveTemp(Row));
    }

    UE_LOG(LogTemp, Warning, TEXT("Loaded %d import/export rows with %d columns"), OutRows.Num(), OutColumnNames.Num());

    return true;
}

FEVFileExchangeResultInfo UEVVocabularyStorageService::GenerateDatabaseExport(EEVFileExtensionType FileExtensionType,
                                                                              TArray<uint8>& OutBytes)
{
    FEVFileExchangeResultInfo ResultInfo;

    OutBytes.Reset();

    switch (FileExtensionType)
    {
    case EEVFileExtensionType::Csv:
    {
        TArray<FString> ColumnNames;
        TArray<FEVDatabaseExportRow> Rows;

        if (!GetImportExportRows(ColumnNames, Rows))
        {
            ResultInfo.Result = EEVFileExchangeResult::DatabaseQueryFailed;
            ResultInfo.UserMessage = TEXT("Failed to read database data for export.");
            ResultInfo.DebugMessage = TEXT("GetImportExportRows returned false.");

            return ResultInfo;
        }

        auto EscapeCsvField = [](const FString& Value) -> FString
        {
            FString EscapedValue = Value;

            EscapedValue.ReplaceInline(TEXT("\""), TEXT("\"\""));

            return FString::Printf(TEXT("\"%s\""), *EscapedValue);
        };

        FString CsvContent;

        CsvContent += FString::Join(ColumnNames, TEXT(","));

        CsvContent += TEXT("\r\n");

        for (const FEVDatabaseExportRow& Row : Rows)
        {
            if (Row.Values.Num() != ColumnNames.Num())
            {
                ResultInfo.Result = EEVFileExchangeResult::InvalidStructure;
                ResultInfo.UserMessage = TEXT("Database export data is inconsistent.");
                ResultInfo.DebugMessage =
                    FString::Printf(TEXT("Export row contains %d values, but %d columns were expected."),
                                    Row.Values.Num(), ColumnNames.Num());

                OutBytes.Reset();
                return ResultInfo;
            }

            TArray<FString> EscapedValues;
            EscapedValues.Reserve(Row.Values.Num());

            for (const FString& Value : Row.Values)
            {
                EscapedValues.Add(EscapeCsvField(Value));
            }

            CsvContent += FString::Join(EscapedValues, TEXT(","));

            CsvContent += TEXT("\r\n");
        }

        const FTCHARToUTF8 Utf8Converter(*CsvContent);

        OutBytes.Append(reinterpret_cast<const uint8*>(Utf8Converter.Get()), Utf8Converter.Length());

        ResultInfo.Result = EEVFileExchangeResult::Success;
        ResultInfo.ByteSize = OutBytes.Num();
        ResultInfo.UserMessage = TEXT("Database export generated successfully.");
        ResultInfo.DebugMessage =
            FString::Printf(TEXT("CSV export generated with %d rows and %d columns."), Rows.Num(), ColumnNames.Num());

        return ResultInfo;
    }

    default:
        ResultInfo.Result = EEVFileExchangeResult::InvalidExtension;
        ResultInfo.UserMessage = TEXT("Unsupported database export format.");
        ResultInfo.DebugMessage = TEXT("GenerateDatabaseExport received an unsupported file extension.");

        return ResultInfo;
    }
}

FEVFileExchangeResultInfo UEVVocabularyStorageService::ValidateImportFile(EEVFileExtensionType FileExtensionType,
                                                                          const TArray<uint8>& Bytes,
                                                                          TArray<uint8>& OutValidationReportBytes)
{
    FEVFileExchangeResultInfo ResultInfo;

    ResultInfo.ByteSize = Bytes.Num();

    OutValidationReportBytes.Reset();

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

    // Remove a UTF-8 BOM when present.
    CsvText.RemoveFromStart(TEXT("\uFEFF"));

    FCsvParser CsvParser(MoveTemp(CsvText));

    const FCsvParser::FRows& ParsedRows = CsvParser.GetRows();

    if (ParsedRows.Num() < 2)
    {
        ResultInfo.Result = EEVFileExchangeResult::EmptyFile;

        ResultInfo.UserMessage = TEXT("The selected import file contains no vocabulary entries.");

        ResultInfo.DebugMessage = FString::Printf(
            TEXT("CSV parser returned %d row(s); expected a header and at least one data row."), ParsedRows.Num());

        return ResultInfo;
    }

    const TArray<FString> ExpectedColumnNames = FEVVocabularyDatabaseSchema::GetImportExportColumnNames();

    if (ExpectedColumnNames.IsEmpty())
    {
        ResultInfo.Result = EEVFileExchangeResult::StorageValidationFailed;

        ResultInfo.UserMessage = TEXT("The database import structure is unavailable.");

        ResultInfo.DebugMessage = TEXT("The vocabulary schema contains no import/export columns.");

        return ResultInfo;
    }

    const TArray<const TCHAR*>& HeaderRow = ParsedRows[0];

    if (HeaderRow.Num() != ExpectedColumnNames.Num())
    {
        ResultInfo.Result = EEVFileExchangeResult::InvalidStructure;

        ResultInfo.UserMessage = TEXT("The selected CSV has an invalid header structure. "
                                      "Please use the downloadable template as a reference.");

        ResultInfo.DebugMessage = FString::Printf(TEXT("CSV header contains %d columns; expected %d."), HeaderRow.Num(),
                                                  ExpectedColumnNames.Num());

        return ResultInfo;
    }

    for (int32 ColumnIndex = 0; ColumnIndex < ExpectedColumnNames.Num(); ++ColumnIndex)
    {
        const FString ActualColumnName = FString(HeaderRow[ColumnIndex]).TrimStartAndEnd();

        const FString& ExpectedColumnName = ExpectedColumnNames[ColumnIndex];

        if (!ActualColumnName.Equals(ExpectedColumnName, ESearchCase::CaseSensitive))
        {
            ResultInfo.Result = EEVFileExchangeResult::InvalidStructure;

            ResultInfo.UserMessage = TEXT("The selected CSV has invalid column names or column order. "
                                          "Please use the downloadable template as a reference.");

            ResultInfo.DebugMessage = FString::Printf(TEXT("CSV column %d is '%s'; expected '%s'."), ColumnIndex,
                                                      *ActualColumnName, *ExpectedColumnName);

            return ResultInfo;
        }
    }

    TArray<FEVValidationFailedEntry> InvalidEntries;
    TArray<FEVValidationFailedEntry> DuplicateEntries;

    TMap<FString, int32> FirstRowByNormalizedWord;
    TSet<FString> ReportedDuplicateWords;

    for (int32 RowIndex = 1; RowIndex < ParsedRows.Num(); ++RowIndex)
    {
        const TArray<const TCHAR*>& Row = ParsedRows[RowIndex];
        const int32 CsvRowNumber = RowIndex + 1;

        if (Row.IsEmpty())
        {
            InvalidEntries.Add({CsvRowNumber, TEXT("<empty row>")});

            continue;
        }

        if (Row.Num() > ExpectedColumnNames.Num())
        {
            ResultInfo.Result = EEVFileExchangeResult::InvalidStructure;

            ResultInfo.UserMessage = TEXT("The selected CSV contains a row with too many columns. "
                                          "Please use the downloadable template as a reference.");

            ResultInfo.DebugMessage =
                FString::Printf(TEXT("CSV row %d contains %d columns; the maximum supported count is %d."),
                                CsvRowNumber, Row.Num(), ExpectedColumnNames.Num());

            return ResultInfo;
        }

        const FString RawWord = FString(Row[0]);

        FString NormalizedWord;
        FText WordValidationError;

        const EEVInputValidationResult WordValidationResult =
            FEVWordInputValidator::ValidateSearchInput(RawWord, NormalizedWord, WordValidationError);

        if (WordValidationResult != EEVInputValidationResult::Valid)
        {
            InvalidEntries.Add({CsvRowNumber, RawWord});

            // Invalid words must not participate in duplicate detection.
            continue;
        }

        if (FirstRowByNormalizedWord.Contains(NormalizedWord))
        {
            if (!ReportedDuplicateWords.Contains(NormalizedWord))
            {
                DuplicateEntries.Add({CsvRowNumber, RawWord});

                ReportedDuplicateWords.Add(NormalizedWord);
            }

            continue;
        }

        FirstRowByNormalizedWord.Add(NormalizedWord, CsvRowNumber);
    }

    if (!InvalidEntries.IsEmpty())
    {
        const FEVFileExchangeResultInfo ReportResult =
            GenerateValidationFailureReport(FileExtensionType, InvalidEntries, OutValidationReportBytes);

        if (!ReportResult.IsSuccess())
        {
            ResultInfo.Result = ReportResult.Result;

            ResultInfo.UserMessage = TEXT("Invalid vocabulary entries were found, "
                                          "but the validation report could not be generated.");

            ResultInfo.DebugMessage = FString::Printf(TEXT("Found %d invalid entries. Report generation failed: %s"),
                                                      InvalidEntries.Num(), *ReportResult.DebugMessage);

            return ResultInfo;
        }

        ResultInfo.Result = EEVFileExchangeResult::InvalidStructure;

        const FString EntryLabel = InvalidEntries.Num() == 1 ? TEXT("entry") : TEXT("entries");

        ResultInfo.UserMessage = FString::Printf(TEXT("The selected CSV contains %d invalid word %s. "
                                                      "Review the generated validation report."),
                                                 InvalidEntries.Num(), *EntryLabel);

        ResultInfo.DebugMessage = FString::Printf(TEXT("CSV validation found %d invalid word entries. "
                                                       "Generated a validation report containing %lld bytes."),
                                                  InvalidEntries.Num(), OutValidationReportBytes.Num());

        return ResultInfo;
    }

    if (!DuplicateEntries.IsEmpty())
    {
        const FEVFileExchangeResultInfo ReportResult =
            GenerateValidationFailureReport(FileExtensionType, DuplicateEntries, OutValidationReportBytes);

        if (!ReportResult.IsSuccess())
        {
            ResultInfo.Result = ReportResult.Result;
            ResultInfo.UserMessage =
                TEXT("Duplicate words were found, but the duplicate report could not be generated.");

            ResultInfo.DebugMessage = FString::Printf(TEXT("Found %d duplicated words. Report generation failed: %s"),
                                                      DuplicateEntries.Num(), *ReportResult.DebugMessage);

            return ResultInfo;
        }

        const FString EntryLabel = DuplicateEntries.Num() == 1 ? TEXT("entry") : TEXT("entries");

        ResultInfo.Result = EEVFileExchangeResult::InvalidStructure;

        ResultInfo.UserMessage = FString::Printf(TEXT("The selected CSV contains %d duplicated word %s. "
                                                      "Review the generated validation report."),
                                                 DuplicateEntries.Num(), *EntryLabel);

        ResultInfo.DebugMessage = FString::Printf(TEXT("CSV validation found %d duplicated normalized words. "
                                                       "Generated a duplicate report containing %d bytes."),
                                                  DuplicateEntries.Num(), OutValidationReportBytes.Num());

        return ResultInfo;
    }

    ResultInfo.Result = EEVFileExchangeResult::Success;

    ResultInfo.UserMessage = TEXT("Import file passed validation.");

    ResultInfo.DebugMessage = FString::Printf(TEXT("CSV validation succeeded for %d data rows."), ParsedRows.Num() - 1);

    return ResultInfo;
}

FEVFileExchangeResultInfo
UEVVocabularyStorageService::GenerateValidationFailureReport(EEVFileExtensionType FileExtensionType,
                                                             const TArray<FEVValidationFailedEntry>& InvalidEntries,
                                                             TArray<uint8>& OutBytes)
{
    FEVFileExchangeResultInfo ResultInfo;

    OutBytes.Reset();

    if (FileExtensionType != EEVFileExtensionType::Csv)
    {
        ResultInfo.Result = EEVFileExchangeResult::InvalidExtension;
        ResultInfo.UserMessage = TEXT("The selected report format is not supported.");
        ResultInfo.DebugMessage = TEXT("GenerateImportProblemReport received a non-CSV extension.");

        return ResultInfo;
    }

    if (InvalidEntries.IsEmpty())
    {
        ResultInfo.Result = EEVFileExchangeResult::EmptyFile;
        ResultInfo.UserMessage = TEXT("There are no problematic entries to report.");
        ResultInfo.DebugMessage = TEXT("GenerateImportProblemReport received no entries.");

        return ResultInfo;
    }

    auto EscapeCsvField = [](const FString& Value) -> FString
    {
        FString Escaped = Value;
        Escaped.ReplaceInline(TEXT("\""), TEXT("\"\""));

        return FString::Printf(TEXT("\"%s\""), *Escaped);
    };

    FString CsvContent = TEXT("RowNumber,ProblematicEntry\r\n");

    for (const FEVValidationFailedEntry& Entry : InvalidEntries)
    {
        CsvContent += FString::Printf(TEXT("%d,%s\r\n"), Entry.RowNumber, *EscapeCsvField(Entry.Entry));
    }

    const FTCHARToUTF8 Utf8Converter(*CsvContent);

    OutBytes.Append(reinterpret_cast<const uint8*>(Utf8Converter.Get()), Utf8Converter.Length());

    ResultInfo.Result = EEVFileExchangeResult::Success;
    ResultInfo.ByteSize = OutBytes.Num();
    ResultInfo.UserMessage = TEXT("Import problem report generated successfully.");
    ResultInfo.DebugMessage =
        FString::Printf(TEXT("Generated CSV report containing %d problematic entries."), InvalidEntries.Num());

    return ResultInfo;
}