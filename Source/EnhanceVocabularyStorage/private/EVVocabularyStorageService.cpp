#include "EVVocabularyStorageService.h"

#include "EVVocabularyDatabaseSchema.h"
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

    // A quote inside a quoted CSV field is represented by two quotes.
    EscapedValue.ReplaceInline(TEXT("\""), TEXT("\"\""));

    return FString::Printf(TEXT("\"%s\""), *EscapedValue);
}
} // namespace

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

    if (!Statement.Create(Database, FEVVocabularySqlQueries::InsertVocabularyEntryStrict,
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

        Entry.bHasUsageExamples = EVVocabularyUsage::HasUsageExamples(Entry.Usage);

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
                                                                          EEVFileOperationType OperationType,
                                                                          const TArray<uint8>& Bytes,
                                                                          TArray<uint8>& OutValidationReportBytes,
                                                                          TArray<FVocabularyEntry>& OutValidatedEntries)
{
    FEVFileExchangeResultInfo ResultInfo;

    ResultInfo.ByteSize = Bytes.Num();

    OutValidationReportBytes.Reset();
    OutValidatedEntries.Reset();

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

        ResultInfo.DebugMessage = FString::Printf(TEXT("CSV parser returned %d row(s); expected a header "
                                                       "and at least one data row."),
                                                  ParsedRows.Num());

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

    /*
     * Header validation
     */

    const TArray<const TCHAR*>& HeaderRow = ParsedRows[0];

    TArray<FString> ActualColumnNames;
    ActualColumnNames.Reserve(HeaderRow.Num());

    for (const TCHAR* HeaderValue : HeaderRow)
    {
        ActualColumnNames.Add(FString(HeaderValue).TrimStartAndEnd());
    }

    TArray<FEVValidationFailedEntry> HeaderProblems;

    /*
     * Find every expected column that is completely absent.
     */
    for (int32 ExpectedIndex = 0; ExpectedIndex < ExpectedColumnNames.Num(); ++ExpectedIndex)
    {
        const FString& ExpectedColumnName = ExpectedColumnNames[ExpectedIndex];

        const int32 ActualPosition = ActualColumnNames.IndexOfByKey(ExpectedColumnName);

        if (ActualPosition != INDEX_NONE)
        {
            continue;
        }

        FEVValidationFailedEntry Problem;
        Problem.Scope = EEVValidationProblemScope::Header;
        Problem.RowNumber = 1;
        Problem.ColumnName = ExpectedColumnName;
        Problem.Entry = TEXT("<missing>");

        Problem.Reason = FString::Printf(TEXT("Required column '%s' is missing. "
                                              "It must appear at position %d."),
                                         *ExpectedColumnName, ExpectedIndex + 1);

        HeaderProblems.Add(MoveTemp(Problem));
    }

    /*
     * Find unexpected, empty, or incorrectly named columns.
     */
    for (int32 ActualIndex = 0; ActualIndex < ActualColumnNames.Num(); ++ActualIndex)
    {
        const FString& ActualColumnName = ActualColumnNames[ActualIndex];

        if (ActualColumnName.IsEmpty())
        {
            FEVValidationFailedEntry Problem;
            Problem.Scope = EEVValidationProblemScope::Header;
            Problem.RowNumber = 1;

            if (ExpectedColumnNames.IsValidIndex(ActualIndex))
            {
                Problem.ColumnName = ExpectedColumnNames[ActualIndex];
            }

            Problem.Entry = TEXT("<empty column name>");

            Problem.Reason = FString::Printf(TEXT("The column name at position %d is empty."), ActualIndex + 1);

            HeaderProblems.Add(MoveTemp(Problem));

            continue;
        }

        if (ExpectedColumnNames.Contains(ActualColumnName))
        {
            continue;
        }

        FEVValidationFailedEntry Problem;
        Problem.Scope = EEVValidationProblemScope::Header;
        Problem.RowNumber = 1;
        Problem.Entry = ActualColumnName;

        if (ExpectedColumnNames.IsValidIndex(ActualIndex))
        {
            Problem.ColumnName = ExpectedColumnNames[ActualIndex];

            Problem.Reason = FString::Printf(TEXT("Incorrect column name '%s' at position %d. "
                                                  "Expected '%s'."),
                                             *ActualColumnName, ActualIndex + 1, *ExpectedColumnNames[ActualIndex]);
        }
        else
        {
            Problem.Reason = FString::Printf(TEXT("Unexpected extra column '%s' at position %d."), *ActualColumnName,
                                             ActualIndex + 1);
        }

        HeaderProblems.Add(MoveTemp(Problem));
    }

    /*
     * Check order only when no columns are missing or unexpected.
     *
     * When the column sets differ, reporting positional problems too
     * would create confusing or contradictory messages.
     */
    if (HeaderProblems.IsEmpty())
    {
        for (int32 ColumnIndex = 0; ColumnIndex < ExpectedColumnNames.Num(); ++ColumnIndex)
        {
            const FString& ExpectedColumnName = ExpectedColumnNames[ColumnIndex];

            const FString& ActualColumnName = ActualColumnNames[ColumnIndex];

            if (ActualColumnName.Equals(ExpectedColumnName, ESearchCase::CaseSensitive))
            {
                continue;
            }

            const int32 CorrectPosition = ExpectedColumnNames.IndexOfByKey(ActualColumnName);

            FEVValidationFailedEntry Problem;
            Problem.Scope = EEVValidationProblemScope::Header;
            Problem.RowNumber = 1;
            Problem.ColumnName = ExpectedColumnName;
            Problem.Entry = ActualColumnName;

            Problem.Reason =
                FString::Printf(TEXT("Column '%s' is in the wrong position. "
                                     "It belongs at position %d, but appears at "
                                     "position %d. Expected '%s' here."),
                                *ActualColumnName, CorrectPosition + 1, ColumnIndex + 1, *ExpectedColumnName);

            HeaderProblems.Add(MoveTemp(Problem));
        }
    }

    if (!HeaderProblems.IsEmpty())
    {
        const FEVFileExchangeResultInfo ReportResult =
            GenerateValidationReport(FileExtensionType, HeaderProblems, OutValidationReportBytes);

        if (!ReportResult.IsSuccess())
        {
            ResultInfo.Result = ReportResult.Result;

            ResultInfo.UserMessage = TEXT("Header problems were found, but the validation "
                                          "report could not be generated.");

            ResultInfo.DebugMessage = FString::Printf(TEXT("Found %d header problems. "
                                                           "Report generation failed: %s"),
                                                      HeaderProblems.Num(), *ReportResult.DebugMessage);

            return ResultInfo;
        }

        ResultInfo.Result = EEVFileExchangeResult::InvalidStructure;

        ResultInfo.UserMessage =
            FString::Printf(TEXT("The selected CSV contains %d header problem%s. "
                                 "Review the generated validation report."),
                            HeaderProblems.Num(), HeaderProblems.Num() == 1 ? TEXT("") : TEXT("s"));

        ResultInfo.DebugMessage = FString::Printf(TEXT("CSV header validation found %d problems. "
                                                       "Generated a report containing %d bytes."),
                                                  HeaderProblems.Num(), OutValidationReportBytes.Num());

        return ResultInfo;
    }

    /*
     * Row validation
     */

    TArray<FEVValidationFailedEntry> RowProblems;

    TMap<FString, int32> FirstRowByNormalizedWord;
    TSet<FString> ReportedDuplicateWords;

    TArray<FVocabularyEntry> ValidatedEntries;
    ValidatedEntries.Reserve(ParsedRows.Num() - 1);

    for (int32 RowIndex = 1; RowIndex < ParsedRows.Num(); ++RowIndex)
    {
        const TArray<const TCHAR*>& Row = ParsedRows[RowIndex];

        const int32 CsvRowNumber = RowIndex + 1;

        if (Row.IsEmpty())
        {
            FEVValidationFailedEntry Problem;
            Problem.Scope = EEVValidationProblemScope::Row;
            Problem.RowNumber = CsvRowNumber;
            Problem.Entry = TEXT("<empty row>");
            Problem.Reason = TEXT("The row is empty.");

            RowProblems.Add(MoveTemp(Problem));

            continue;
        }

        if (Row.Num() > ExpectedColumnNames.Num())
        {
            FEVValidationFailedEntry Problem;
            Problem.Scope = EEVValidationProblemScope::Row;
            Problem.RowNumber = CsvRowNumber;
            Problem.Entry = FString::Printf(TEXT("%d columns"), Row.Num());
            Problem.Reason = FString::Printf(TEXT("The row contains %d columns, but only %d columns are expected."),
                                             Row.Num(), ExpectedColumnNames.Num());

            RowProblems.Add(MoveTemp(Problem));

            continue;
        }

        const FString RawWord = FString(Row[0]);

        FString NormalizedWord;
        FText WordValidationError;

        const EEVInputValidationResult WordValidationResult =
            FEVWordInputValidator::ValidateSearchInput(RawWord, NormalizedWord, WordValidationError);

        if (WordValidationResult != EEVInputValidationResult::Valid)
        {
            FEVValidationFailedEntry Problem;
            Problem.Scope = EEVValidationProblemScope::Row;
            Problem.RowNumber = CsvRowNumber;
            Problem.ColumnName = TEXT("Word");
            Problem.Entry = RawWord;
            Problem.Reason = WordValidationError.ToString();

            RowProblems.Add(MoveTemp(Problem));

            // Invalid words must not participate in duplicate detection.
            continue;
        }

        if (const int32* FirstRowNumber = FirstRowByNormalizedWord.Find(NormalizedWord))
        {
            if (!ReportedDuplicateWords.Contains(NormalizedWord))
            {
                FEVValidationFailedEntry Problem;
                Problem.Scope = EEVValidationProblemScope::Row;

                Problem.RowNumber = CsvRowNumber;
                Problem.ColumnName = TEXT("Word");
                Problem.Entry = RawWord;

                Problem.Reason = FString::Printf(TEXT("Duplicate word inside the import file. "
                                                      "The first occurrence is on row %d."),
                                                 *FirstRowNumber);

                RowProblems.Add(MoveTemp(Problem));

                ReportedDuplicateWords.Add(NormalizedWord);
            }

            continue;
        }

        FirstRowByNormalizedWord.Add(NormalizedWord, CsvRowNumber);

        FVocabularyEntry Entry;
        Entry.Word = NormalizedWord;

        if (Row.Num() > 1)
        {
            Entry.Definition = FString(Row[1]);
        }

        if (Row.Num() > 2)
        {
            Entry.Usage = FString(Row[2]);
        }

        if (Row.Num() > 3)
        {
            Entry.TranslationRu = FString(Row[3]);
        }

        if (Row.Num() > 4)
        {
            Entry.TranslationUa = FString(Row[4]);
        }

        ValidatedEntries.Add(MoveTemp(Entry));
    }

    if (!RowProblems.IsEmpty())
    {
        const FEVFileExchangeResultInfo ReportResult =
            GenerateValidationReport(FileExtensionType, RowProblems, OutValidationReportBytes);

        if (!ReportResult.IsSuccess())
        {
            ResultInfo.Result = ReportResult.Result;

            ResultInfo.UserMessage = TEXT("Import-file problems were found, but the validation "
                                          "report could not be generated.");

            ResultInfo.DebugMessage = FString::Printf(TEXT("Found %d row-level problems. "
                                                           "Report generation failed: %s"),
                                                      RowProblems.Num(), *ReportResult.DebugMessage);

            return ResultInfo;
        }

        ResultInfo.Result = EEVFileExchangeResult::InvalidStructure;

        ResultInfo.UserMessage = FString::Printf(TEXT("The selected CSV contains %d row-level problem%s. "
                                                      "Review the generated validation report."),
                                                 RowProblems.Num(), RowProblems.Num() == 1 ? TEXT("") : TEXT("s"));

        ResultInfo.DebugMessage = FString::Printf(TEXT("CSV row validation found %d problems. "
                                                       "Generated a validation report containing %d bytes."),
                                                  RowProblems.Num(), OutValidationReportBytes.Num());

        return ResultInfo;
    }

    if (OperationType == EEVFileOperationType::ImportDBAppend)
    {
        TArray<FEVValidationFailedEntry> DatabaseProblems;

        CollectAppendValidationProblems(ValidatedEntries, DatabaseProblems);

        if (!DatabaseProblems.IsEmpty())
        {
            const FEVFileExchangeResultInfo ReportResult =
                GenerateValidationReport(FileExtensionType, DatabaseProblems, OutValidationReportBytes);

            if (!ReportResult.IsSuccess())
            {
                ResultInfo.Result = ReportResult.Result;

                ResultInfo.UserMessage = TEXT("Database conflicts were found, but the validation "
                                              "report could not be generated.");

                ResultInfo.DebugMessage = FString::Printf(TEXT("Found %d append database problems. "
                                                               "Report generation failed: %s"),
                                                          DatabaseProblems.Num(), *ReportResult.DebugMessage);

                return ResultInfo;
            }

            ResultInfo.Result = EEVFileExchangeResult::InvalidStructure;

            ResultInfo.UserMessage =
                FString::Printf(TEXT("The selected CSV contains %d database conflict%s. "
                                     "Review the generated validation report."),
                                DatabaseProblems.Num(), DatabaseProblems.Num() == 1 ? TEXT("") : TEXT("s"));

            ResultInfo.DebugMessage = FString::Printf(TEXT("Append validation found %d database problems. "
                                                           "Generated a validation report containing %d bytes."),
                                                      DatabaseProblems.Num(), OutValidationReportBytes.Num());

            return ResultInfo;
        }
    }

    OutValidatedEntries = MoveTemp(ValidatedEntries);

    ResultInfo.Result = EEVFileExchangeResult::Success;
    ResultInfo.UserMessage = TEXT("Import file passed validation.");

    ResultInfo.DebugMessage = FString::Printf(TEXT("CSV validation succeeded and produced %d "
                                                   "vocabulary entries."),
                                              OutValidatedEntries.Num());

    return ResultInfo;
}

FEVFileExchangeResultInfo UEVVocabularyStorageService::OverwriteDatabase(const TArray<FVocabularyEntry>& Entries)
{
    FEVFileExchangeResultInfo ResultInfo;

    if (!Database.IsValid())
    {
        ResultInfo.Result = EEVFileExchangeResult::DatabaseTransactionFailed;

        ResultInfo.UserMessage = TEXT("The database is unavailable.");

        ResultInfo.DebugMessage = TEXT("OverwriteDatabase received an invalid database.");

        return ResultInfo;
    }

    if (Entries.IsEmpty())
    {
        ResultInfo.Result = EEVFileExchangeResult::EmptyFile;

        ResultInfo.UserMessage = TEXT("There are no vocabulary entries to import.");

        ResultInfo.DebugMessage = TEXT("OverwriteDatabase received an empty entry array.");

        return ResultInfo;
    }

    if (!Database.Execute(TEXT("BEGIN TRANSACTION;")))
    {
        ResultInfo.Result = EEVFileExchangeResult::DatabaseTransactionFailed;

        ResultInfo.UserMessage = TEXT("The database overwrite could not be started.");

        ResultInfo.DebugMessage = TEXT("Failed to begin the overwrite transaction.");

        return ResultInfo;
    }

    auto RollbackTransaction = [this]()
    {
        if (!Database.Execute(TEXT("ROLLBACK;")))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to roll back the database overwrite transaction."));
        }
    };

    const FString DeleteAllQuery =
        FString::Printf(TEXT("DELETE FROM %s;"), *FEVVocabularyDatabaseSchema::GetTableName());

    if (!Database.Execute(*DeleteAllQuery))
    {
        RollbackTransaction();

        ResultInfo.Result = EEVFileExchangeResult::DatabaseTransactionFailed;

        ResultInfo.UserMessage = TEXT("The current database could not be cleared.");

        ResultInfo.DebugMessage = FString::Printf(TEXT("Failed to delete existing rows from table '%s'."),
                                                  *FEVVocabularyDatabaseSchema::GetTableName());

        return ResultInfo;
    }

    for (int32 EntryIndex = 0; EntryIndex < Entries.Num(); ++EntryIndex)
    {
        const FVocabularyEntry& Entry = Entries[EntryIndex];

        if (!InsertVocabularyEntryStrict(Entry))
        {
            RollbackTransaction();

            ResultInfo.Result = EEVFileExchangeResult::DatabaseTransactionFailed;

            ResultInfo.UserMessage = TEXT("The imported entries could not be written to the database.");

            ResultInfo.DebugMessage = FString::Printf(TEXT("Failed to insert imported entry %d of %d. Word: '%s'."),
                                                      EntryIndex + 1, Entries.Num(), *Entry.Word);

            return ResultInfo;
        }
    }

    if (!Database.Execute(TEXT("COMMIT;")))
    {
        RollbackTransaction();

        ResultInfo.Result = EEVFileExchangeResult::DatabaseTransactionFailed;

        ResultInfo.UserMessage = TEXT("The database overwrite could not be completed.");

        ResultInfo.DebugMessage = TEXT("Failed to commit the overwrite transaction.");

        return ResultInfo;
    }

    ResultInfo.Result = EEVFileExchangeResult::Success;

    ResultInfo.UserMessage = TEXT("The database was overwritten successfully.");

    ResultInfo.DebugMessage =
        FString::Printf(TEXT("Database overwrite committed successfully with %d entries."), Entries.Num());

    return ResultInfo;
}

FEVFileExchangeResultInfo UEVVocabularyStorageService::AppendDatabase(const TArray<FVocabularyEntry>& Entries)
{
    FEVFileExchangeResultInfo ResultInfo;

    if (!Database.IsValid())
    {
        ResultInfo.Result = EEVFileExchangeResult::DatabaseTransactionFailed;

        ResultInfo.UserMessage = TEXT("The database is unavailable.");

        ResultInfo.DebugMessage = TEXT("AppendDatabase received an invalid database.");

        return ResultInfo;
    }

    if (Entries.IsEmpty())
    {
        ResultInfo.Result = EEVFileExchangeResult::EmptyFile;

        ResultInfo.UserMessage = TEXT("There are no vocabulary entries to append.");

        ResultInfo.DebugMessage = TEXT("AppendDatabase received an empty entry array.");

        return ResultInfo;
    }

    /*
     * No duplicates were found against the current database.
     * Append every validated entry atomically.
     */
    if (!Database.Execute(TEXT("BEGIN TRANSACTION;")))
    {
        ResultInfo.Result = EEVFileExchangeResult::DatabaseTransactionFailed;

        ResultInfo.UserMessage = TEXT("The database append could not be started.");

        ResultInfo.DebugMessage = TEXT("AppendDatabase failed to begin the append transaction.");

        return ResultInfo;
    }

    auto RollbackTransaction = [this]()
    {
        if (!Database.Execute(TEXT("ROLLBACK;")))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to roll back the database append transaction."));
        }
    };

    for (int32 EntryIndex = 0; EntryIndex < Entries.Num(); ++EntryIndex)
    {
        const FVocabularyEntry& Entry = Entries[EntryIndex];

        if (!InsertVocabularyEntryStrict(Entry))
        {
            RollbackTransaction();

            ResultInfo.Result = EEVFileExchangeResult::DatabaseTransactionFailed;

            ResultInfo.UserMessage = TEXT("The imported entries could not be appended to the database.");

            ResultInfo.DebugMessage =
                FString::Printf(TEXT("AppendDatabase failed to insert entry %d of %d. Word: '%s'."), EntryIndex + 1,
                                Entries.Num(), *Entry.Word);

            return ResultInfo;
        }
    }

    if (!Database.Execute(TEXT("COMMIT;")))
    {
        RollbackTransaction();

        ResultInfo.Result = EEVFileExchangeResult::DatabaseTransactionFailed;

        ResultInfo.UserMessage = TEXT("The database append could not be completed.");

        ResultInfo.DebugMessage = TEXT("AppendDatabase failed to commit the append transaction.");

        return ResultInfo;
    }

    ResultInfo.Result = EEVFileExchangeResult::Success;

    ResultInfo.UserMessage = TEXT("The imported entries were appended successfully.");

    ResultInfo.DebugMessage =
        FString::Printf(TEXT("Append transaction committed successfully with %d new entries."), Entries.Num());

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

bool UEVVocabularyStorageService::InsertVocabularyEntryStrict(const FVocabularyEntry& Entry)
{
    if (!Database.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot strictly insert entry: database is invalid."));

        return false;
    }

    FSQLitePreparedStatement Statement;

    if (!Statement.Create(Database, FEVVocabularySqlQueries::InsertVocabularyEntryStrict,
                          ESQLitePreparedStatementFlags::Persistent))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create strict INSERT statement for word: %s"), *Entry.Word);

        return false;
    }

    Statement.SetBindingValueByIndex(1, Entry.Word);
    Statement.SetBindingValueByIndex(2, Entry.Definition);
    Statement.SetBindingValueByIndex(3, Entry.Usage);
    Statement.SetBindingValueByIndex(4, Entry.TranslationRu);
    Statement.SetBindingValueByIndex(5, Entry.TranslationUa);

    if (!Statement.Execute())
    {
        UE_LOG(LogTemp, Error, TEXT("Strict INSERT failed for vocabulary entry: %s"), *Entry.Word);

        return false;
    }

    return true;
}

void UEVVocabularyStorageService::CollectAppendValidationProblems(const TArray<FVocabularyEntry>& Entries,
                                                                  TArray<FEVValidationFailedEntry>& OutProblems)
{
    OutProblems.Reset();

    if (!Database.IsValid())
    {
        FEVValidationFailedEntry Problem;
        Problem.Scope = EEVValidationProblemScope::Database;
        Problem.Reason = TEXT("The current database is unavailable.");

        OutProblems.Add(MoveTemp(Problem));
        return;
    }

    FSQLitePreparedStatement Statement;

    if (!Statement.Create(Database, FEVVocabularySqlQueries::SelectVocabularyEntries,
                          ESQLitePreparedStatementFlags::Persistent))
    {
        FEVValidationFailedEntry Problem;
        Problem.Scope = EEVValidationProblemScope::Database;
        Problem.Reason = TEXT("The current database entries could not be checked.");

        OutProblems.Add(MoveTemp(Problem));
        return;
    }

    TSet<FString> ExistingNormalizedWords;

    while (Statement.Step() == ESQLitePreparedStatementStepResult::Row)
    {
        FString ExistingWord;

        if (!Statement.GetColumnValueByIndex(0, ExistingWord))
        {
            FEVValidationFailedEntry Problem;
            Problem.Scope = EEVValidationProblemScope::Database;
            Problem.Reason = TEXT("An existing Word value could not be read from the database.");

            OutProblems.Add(MoveTemp(Problem));
            return;
        }

        ExistingNormalizedWords.Add(FEVWordInputValidator::NormalizeWordInput(ExistingWord));
    }

    for (int32 EntryIndex = 0; EntryIndex < Entries.Num(); ++EntryIndex)
    {
        const FVocabularyEntry& Entry = Entries[EntryIndex];

        const FString NormalizedImportedWord = FEVWordInputValidator::NormalizeWordInput(Entry.Word);

        if (!ExistingNormalizedWords.Contains(NormalizedImportedWord))
        {
            continue;
        }

        FEVValidationFailedEntry Problem;
        Problem.Scope = EEVValidationProblemScope::Database;
        Problem.RowNumber = EntryIndex + 2;
        Problem.ColumnName = TEXT("Word");
        Problem.Entry = Entry.Word;
        Problem.Reason = TEXT("This word already exists in the current database.");

        OutProblems.Add(MoveTemp(Problem));
    }
}