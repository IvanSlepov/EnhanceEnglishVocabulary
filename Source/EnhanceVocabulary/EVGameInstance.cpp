// Fill out your copyright notice in the Description page of Project Settings.

#include "EVGameInstance.h"

#include "EVFileExchangeDefaults.h"

#include "EVVocabularyStorageService.h"
#include "EVWordSearchService.h"
#include "EVConnectivityService.h"
#include "EVDeviceService.h"

void UEVGameInstance::Init()
{
    Super::Init();

    VocabularyStorageService = NewObject<UEVVocabularyStorageService>(this);

    WordSearchService = NewObject<UEVWordSearchService>(this);

    ConnectivityService = NewObject<UEVConnectivityService>(this);

    DeviceService = NewObject<UEVDeviceService>(this);

    if (WordSearchService)
    {
        WordSearchService->Initialize();

        WordSearchService->OnEVWordSearchCompleted.AddDynamic(
            this, &ThisClass::HandleEVWordSearchCompletedFromEVGameInstance);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("WordSearchService is null"));
    }

    if (ConnectivityService)
    {
        ConnectivityService->Initialize();
        ConnectivityService->RefreshConnection();

        ConnectivityService->OnConnectionStateChanged.AddDynamic(this, &ThisClass::HandleConnectionStateChanged);

        if (UWorld* World = GetWorld())
        {
            ConnectivityService->StartConnectionPolling(World);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("World is null; connection polling not started"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ConnectivityService is null"));
    }

    if (VocabularyStorageService)
    {
        VocabularyStorageService->InitializeStorage();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));
    }

    if (DeviceService)
    {
        DeviceService->InitializeDeviceService();

        DeviceService->OnFileSaved().AddUObject(this, &ThisClass::HandleFileSaved);

        DeviceService->OnImportFilePicked().AddUObject(this, &ThisClass::HandleImportFilePicked);

        DeviceService->OnPopUpTimerExpired.AddUObject(this, &ThisClass::HandlePopUpTimerExpired);

        DeviceService->OnNotificationPermissionResult().AddUObject(this,
                                                                   &ThisClass::HandleNotificationPermissionResult);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DeviceService is null"));
    }
}

void UEVGameInstance::Shutdown()
{
    if (VocabularyStorageService)
    {
        VocabularyStorageService->ShutdownStorage();
    }

    if (ConnectivityService)
    {
        if (UWorld* World = GetWorld())
        {
            ConnectivityService->StopConnectionPolling(World);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("World is null; connection polling not stopped"));
        }

        ConnectivityService->Shutdown();
    }

    Super::Shutdown();
}

EEVVocabularyStorageServiceResult UEVGameInstance::DoesWordExist(const FString& Word, FText& OutErrorMessage)
{
    OutErrorMessage = FText::GetEmpty();

    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));

        return EEVVocabularyStorageServiceResult::VocabularyStorageInstanceError;
    }

    switch (VocabularyStorageService->DoesWordExist(Word, OutErrorMessage))
    {
    case EEVWordLookupResult::DatabaseError:
        return EEVVocabularyStorageServiceResult::DatabaseError;

    case EEVWordLookupResult::Exists:
        return EEVVocabularyStorageServiceResult::WordExists;

    case EEVWordLookupResult::DoesNotExist:
        return EEVVocabularyStorageServiceResult::WordDoesNotExist;

    default:
        break;
    }

    return EEVVocabularyStorageServiceResult::Empty;
}

bool UEVGameInstance::SaveVocabularyEntry(const FWordSearchResult& WordSearchResult)
{
    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));

        return false;
    }

    FVocabularyEntry Entry;
    Entry.Word = WordSearchResult.Word;
    Entry.Transcription = WordSearchResult.Transcription;
    Entry.Definition = WordSearchResult.Definition;
    Entry.Usage = WordSearchResult.Usage;
    Entry.TranslationRu = WordSearchResult.TranslationRu;
    Entry.TranslationUa = WordSearchResult.TranslationUa;

    return VocabularyStorageService->SaveVocabularyEntry(Entry);
}

bool UEVGameInstance::UpdateVocabularyEntry(const FVocabularyEntry& Entry, FVocabularyEntry& OutEntry)
{
    OutEntry = FVocabularyEntry();

    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));

        return false;
    }

    if (!VocabularyStorageService->UpdateVocabularyEntry(Entry))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to update vocabulary entry: %s"), *Entry.Word);

        return false;
    }

    return VocabularyStorageService->GetVocabularyEntryByWord(Entry.Word, OutEntry);
}

bool UEVGameInstance::GetVocabularyEntryByWord(const FString& Word, FVocabularyEntry& OutEntry) const
{
    OutEntry = FVocabularyEntry();
    return VocabularyStorageService && VocabularyStorageService->GetVocabularyEntryByWord(Word, OutEntry);
}

bool UEVGameInstance::DeleteVocabularyEntry(const FVocabularyEntry& Entry)
{
    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));

        return false;
    }

    if (!VocabularyStorageService->DeleteVocabularyEntry(Entry))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to delete vocabulary entry: %s"), *Entry.Word);

        return false;
    }

    return true;
}

int32 UEVGameInstance::GetVocabularyEntryCount() const
{
    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("GetVocabularyEntryCount: VocabularyStorageService is null"));

        return 0;
    }

    return VocabularyStorageService->GetVocabularyEntryCount();
}

bool UEVGameInstance::GetVocabularyEntriesPage(TArray<FVocabularyEntry>& OutVocabularyEntries, int32 Limit,
                                               int32 Offset) const
{
    OutVocabularyEntries.Reset();

    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("GetVocabularyEntriesPage: VocabularyStorageService is null"));

        return false;
    }

    if (Limit <= 0 || Offset < 0)
    {
        UE_LOG(LogTemp, Error, TEXT("GetVocabularyEntriesPage received invalid arguments. Limit=%d | Offset=%d"), Limit,
               Offset);

        return false;
    }

    OutVocabularyEntries = VocabularyStorageService->GetVocabularyEntriesPage(Limit, Offset);

    return true;
}

bool UEVGameInstance::GetVocabularyEntries(TArray<FVocabularyEntry>& OutVocabularyEntries, int32 EntryNumber)
{
    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));

        return false;
    }

    OutVocabularyEntries = VocabularyStorageService->GetVocabularyEntries(EntryNumber);

    return true;
}

int32 UEVGameInstance::GetVocabularyEntryCountByPrefix(const FString& SearchPrefix) const
{
    if (!VocabularyStorageService)
    {
        return 0;
    }

    return VocabularyStorageService->GetVocabularyEntryCountByPrefix(SearchPrefix);
}

bool UEVGameInstance::GetVocabularyEntriesPageByPrefix(TArray<FVocabularyEntry>& OutVocabularyEntries,
                                                       const FString& SearchPrefix, int32 Limit, int32 Offset) const
{
    OutVocabularyEntries.Reset();

    if (!VocabularyStorageService)
    {
        return false;
    }

    OutVocabularyEntries = VocabularyStorageService->GetVocabularyEntriesPageByPrefix(SearchPrefix, Limit, Offset);

    return true;
}

bool UEVGameInstance::GetRandomlySelectedWord(FString& OutWord)
{
    OutWord.Reset();

    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot retrieve random word: vocabulary storage service is invalid."));

        return false;
    }

    return VocabularyStorageService->GetRandomlySelectedWord(OutWord);
}

bool UEVGameInstance::HandlePopUpIntervalSelected(const FEVPopUpSettingsInfo& PopUpSettings)
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot apply notification settings: DeviceService is invalid."));
        return false;
    }

    CurrentPopUpSettings = PopUpSettings;

    const int32 IntervalSeconds = FEVPopUpSettingsInfo::GetIntervalSeconds(PopUpSettings.PopUpIntervals);

    if (IntervalSeconds <= 0)
    {
        return DeviceService->CancelVocabularyNotifications();
    }

#if PLATFORM_ANDROID
    switch (PopUpSettings.NotificationMode)
    {
    case EEVNotificationMode::RandomWord:
    {
        if (!VocabularyStorageService)
        {
            UE_LOG(LogTemp, Error, TEXT("Cannot schedule RandomWord notifications: vocabulary storage is invalid."));
            return false;
        }

        const int32 EntryCount = VocabularyStorageService->GetVocabularyEntryCount();
        if (EntryCount <= 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot schedule RandomWord notifications: vocabulary database is empty."));
            return false;
        }

        const TArray<FVocabularyEntry> Entries = VocabularyStorageService->GetVocabularyEntries(EntryCount);
        TArray<FString> Words;
        Words.Reserve(Entries.Num());

        for (const FVocabularyEntry& Entry : Entries)
        {
            FString Word = Entry.Word;
            Word.ReplaceInline(TEXT("\r"), TEXT(" "));
            Word.ReplaceInline(TEXT("\n"), TEXT(" "));
            Word.TrimStartAndEndInline();

            if (!Word.IsEmpty())
            {
                Words.Add(MoveTemp(Word));
            }
        }

        if (Words.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot schedule RandomWord notifications: no valid words exist."));
            return false;
        }

        return DeviceService->ScheduleVocabularyNotifications(IntervalSeconds, FString::Join(Words, TEXT("\n")),
                                                              static_cast<int32>(PopUpSettings.NotificationMode));
    }

    case EEVNotificationMode::TestMode:
        return DeviceService->ScheduleVocabularyNotifications(IntervalSeconds, TEXT(""),
                                                              static_cast<int32>(PopUpSettings.NotificationMode));

    default:
        ensureMsgf(false, TEXT("Unsupported notification mode."));
        return false;
    }
#else
    return DeviceService->StartPopUpTimer(IntervalSeconds);
#endif
}

FWordSearchResult UEVGameInstance::SearchWordFake(const FString& Word)
{
    if (!WordSearchService)
    {
        FWordSearchResult Result;
        Result.bSuccess = false;
        Result.ErrorMessage = TEXT("WordSearchService/Fake search is null");

        return Result;
    }

    return WordSearchService->SearchWordFake(Word);
}

void UEVGameInstance::SearchWordOnline(const FString& Word, EEVWebProvider DefinitionUsageProvider,
                                       EEVWebProvider TranslationProvider)
{
    if (!WordSearchService)
    {
        UE_LOG(LogTemp, Error, TEXT("WordSearchService/Real Online Search is null"));

        return;
    }

    WordSearchService->SearchWordOnline(Word, DefinitionUsageProvider, TranslationProvider);
}

FEVRequestedActionInfo UEVGameInstance::HandleFileOperationRequested(const FEVFileOperationInfo& FileOperationInfo)
{
    switch (FileOperationInfo.OperationType)
    {
    case EEVFileOperationType::DownloadTemplate:
        return HandleDownloadTemplateRequested(FileOperationInfo);

    case EEVFileOperationType::ExportDB:
        return HandleExportDBRequested(FileOperationInfo);

    case EEVFileOperationType::ImportDBOverwrite:
        return HandleImportDBOverwriteRequested(FileOperationInfo);

    case EEVFileOperationType::ImportDBAppend:
        return HandleImportDBAppendRequested(FileOperationInfo);

    default:
    {
        FEVRequestedActionInfo ActionInfo;
        ActionInfo.Source = EEVRequestedActionSource::ImportExport;
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("Unsupported file operation."));
        ActionInfo.GenerateColor();

        return ActionInfo;
    }
    }
}

EEVConnectionState UEVGameInstance::GetConnectionState() const
{
    if (ConnectivityService)
    {
        return ConnectivityService->GetConnectionState();
    }

    UE_LOG(LogTemp, Error, TEXT("ConnectivityService is null in UEVGameInstance::GetConnectionState()"));

    return EEVConnectionState::Offline;
}

void UEVGameInstance::HandleConnectionStateChanged(EEVConnectionState NewState)
{
    if (EVConnectionState == NewState)
    {
        return;
    }

    EVConnectionState = NewState;

    OnConnectionStateChanged.Broadcast(NewState);
}

void UEVGameInstance::HandleEVWordSearchCompletedFromEVGameInstance(
    const FWordSearchResult& SearchWordResultPassedByGameInstance)
{
    OnEVWordSearchCompletedFromEVGameInstance.Broadcast(SearchWordResultPassedByGameInstance);
}

void UEVGameInstance::HandlePopUpTimerExpired()
{
    FString RandomWord;

    if (!VocabularyStorageService->GetRandomlySelectedWord(RandomWord))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to retrieve a random word for the notification."));

        return;
    }

    if (!DeviceService->ShowVocabularyNotification(RandomWord))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to show vocabulary notification."));

        return;
    }

    const int32 IntervalSeconds = FEVPopUpSettingsInfo::GetIntervalSeconds(CurrentPopUpSettings.PopUpIntervals);

    if (IntervalSeconds > 0)
    {
        DeviceService->StartPopUpTimer(IntervalSeconds);
    }
}

void UEVGameInstance::HandleFileSaved(const FEVFileExchangeResultInfo& ResultInfo)
{
    UE_LOG(LogTemp, Log, TEXT("File saved result: %d | File: %s | Bytes: %lld | Message: %s | Debug: %s"),
           static_cast<int32>(ResultInfo.Result), *ResultInfo.FileName, ResultInfo.ByteSize, *ResultInfo.UserMessage,
           *ResultInfo.DebugMessage);

    /*
     * A successfully saved validation report does not mean that
     * the database import succeeded.
     *
     * The import remains failed because its source data did not
     * pass validation.
     */
    if (PendingFileSavePurpose == EEVPendingFileSavePurpose::ImportValidationReport)
    {
        FEVRequestedActionInfo ActionInfo;
        ActionInfo.Source = EEVRequestedActionSource::ImportExport;

        switch (PendingImportFileOperationInfo.OperationType)
        {
        case EEVFileOperationType::ImportDBAppend:
            ActionInfo.Type = EEVRequestedActionType::ImportDBAppend;
            break;

        case EEVFileOperationType::ImportDBOverwrite:
        default:
            ActionInfo.Type = EEVRequestedActionType::Unknown;
            break;
        }

        ActionInfo.Status = EEVRequestedActionStatus::Failed;

        if (ResultInfo.IsSuccess())
        {
            ActionInfo.Message = FText::FromString(PendingImportValidationResult.UserMessage +
                                                   TEXT(" The validation report was saved successfully."));
        }
        else if (ResultInfo.Result == EEVFileExchangeResult::CancelledByUser)
        {
            ActionInfo.Message = FText::FromString(PendingImportValidationResult.UserMessage +
                                                   TEXT(" Saving the validation report was cancelled."));
        }
        else
        {
            ActionInfo.Message = FText::FromString(PendingImportValidationResult.UserMessage +
                                                   TEXT(" The validation report could not be saved."));
        }

        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Warning,
               TEXT("Import validation failed. Report save result: %d | Validation debug: %s | Save debug: %s"),
               static_cast<int32>(ResultInfo.Result), *PendingImportValidationResult.DebugMessage,
               *ResultInfo.DebugMessage);

        PendingFileSavePurpose = EEVPendingFileSavePurpose::None;

        PendingImportValidationResult = FEVFileExchangeResultInfo();

        PendingImportFileOperationInfo = FEVFileOperationInfo();

        FileOperationCompletedDelegate.Broadcast(ActionInfo);

        return;
    }

    /*
     * Normal Template / Export completion.
     */
    FEVFileExchangeResultInfo AdjustedResult = ResultInfo;

    if (ResultInfo.IsSuccess())
    {
        switch (PendingFileSavePurpose)
        {
        case EEVPendingFileSavePurpose::DownloadTemplate:
            AdjustedResult.UserMessage = TEXT("The database template was saved successfully.");
            break;

        case EEVPendingFileSavePurpose::ExportDatabase:
            AdjustedResult.UserMessage = TEXT("The database export was saved successfully.");
            break;

        default:
            break;
        }
    }

    PendingFileSavePurpose = EEVPendingFileSavePurpose::None;

    const FEVRequestedActionInfo ActionInfo = ConvertFileExchangeResultToRequestedAction(AdjustedResult);

    FileOperationCompletedDelegate.Broadcast(ActionInfo);
}

void UEVGameInstance::HandleImportFilePicked(const FEVFileExchangeResultInfo& ResultInfo, const TArray<uint8>& Bytes)
{
    UE_LOG(LogTemp, Log,
           TEXT("Import file picked result: %d | File: %s | Bytes: %lld | ReceivedBytes: %d | Message: %s | Debug: %s"),
           static_cast<int32>(ResultInfo.Result), *ResultInfo.FileName, ResultInfo.ByteSize, Bytes.Num(),
           *ResultInfo.UserMessage, *ResultInfo.DebugMessage);

    if (!ResultInfo.IsSuccess())
    {
        CompleteImportFileOperation(ResultInfo);
        return;
    }

    if (!VocabularyStorageService)
    {
        FEVFileExchangeResultInfo ErrorResult;
        ErrorResult.Result = EEVFileExchangeResult::StorageValidationFailed;
        ErrorResult.UserMessage = TEXT("Vocabulary storage service is not available.");
        ErrorResult.DebugMessage = TEXT("HandleImportFilePicked: VocabularyStorageService is null.");

        PopulateImportResultFileInfo(ErrorResult, ResultInfo, Bytes.Num());

        CompleteImportFileOperation(ErrorResult);
        return;
    }

    TArray<uint8> ValidationReportBytes;
    TArray<FVocabularyEntry> ValidatedEntries;

    FEVFileExchangeResultInfo ValidationResult = VocabularyStorageService->ValidateImportFile(
        PendingImportFileOperationInfo.FileExtensionType, PendingImportFileOperationInfo.OperationType, Bytes,
        ValidationReportBytes, ValidatedEntries);

    PopulateImportResultFileInfo(ValidationResult, ResultInfo, Bytes.Num());

    UE_LOG(LogTemp, Log, TEXT("Import validation result: %d | ReportBytes: %d | Message: %s | Debug: %s"),
           static_cast<int32>(ValidationResult.Result), ValidationReportBytes.Num(), *ValidationResult.UserMessage,
           *ValidationResult.DebugMessage);

    if (TrySaveImportValidationReport(ValidationResult, ValidationReportBytes))
    {
        return;
    }

    if (!ValidationResult.IsSuccess())
    {
        CompleteImportFileOperation(ValidationResult);
        return;
    }

    FEVFileExchangeResultInfo DatabaseOperationResult = ExecuteImportDatabaseOperation(ValidatedEntries);

    PopulateImportResultFileInfo(DatabaseOperationResult, ResultInfo, Bytes.Num());

    UE_LOG(LogTemp, Log, TEXT("Import database operation result: %d | ReportBytes: %d | Message: %s | Debug: %s"),
           static_cast<int32>(DatabaseOperationResult.Result), ValidationReportBytes.Num(),
           *DatabaseOperationResult.UserMessage, *DatabaseOperationResult.DebugMessage);

    if (TrySaveImportValidationReport(DatabaseOperationResult, ValidationReportBytes))
    {
        return;
    }

    CompleteImportFileOperation(DatabaseOperationResult);
}

void UEVGameInstance::CompleteImportFileOperation(const FEVFileExchangeResultInfo& ResultInfo)
{
    PendingImportFileOperationInfo = FEVFileOperationInfo();

    ImportFilePickCompletedDelegate.Broadcast(ResultInfo);
}

bool UEVGameInstance::TrySaveImportValidationReport(FEVFileExchangeResultInfo ValidationResult,
                                                    const TArray<uint8>& ValidationReportBytes)
{
    if (ValidationReportBytes.IsEmpty())
    {
        return false;
    }

    if (!DeviceService)
    {
        ValidationResult.Result = EEVFileExchangeResult::UnsupportedPlatform;

        ValidationResult.UserMessage = TEXT("The validation report could not be saved.");

        ValidationResult.DebugMessage = TEXT("TrySaveImportValidationReport: DeviceService is null.");

        CompleteImportFileOperation(ValidationResult);

        return true;
    }

    PendingImportValidationResult = ValidationResult;

    PendingFileSavePurpose = EEVPendingFileSavePurpose::ImportValidationReport;

    UE_LOG(LogTemp, Warning, TEXT("Validation report save requested. Bytes: %d | Validation: %s"),
           ValidationReportBytes.Num(), *ValidationResult.DebugMessage);

    DeviceService->SaveBytesToUserSelectedLocation(
        PendingImportFileOperationInfo.FileExtensionType,
        FEVFileExchangeDefaults::GetValidationReportFileName(PendingImportFileOperationInfo.FileExtensionType),
        ValidationReportBytes);

    return true;
}

FEVFileExchangeResultInfo
UEVGameInstance::ExecuteImportDatabaseOperation(const TArray<FVocabularyEntry>& ValidatedEntries)
{
    switch (PendingImportFileOperationInfo.OperationType)
    {
    case EEVFileOperationType::ImportDBOverwrite:
        return VocabularyStorageService->OverwriteDatabase(ValidatedEntries);

    case EEVFileOperationType::ImportDBAppend:
        return VocabularyStorageService->AppendDatabase(ValidatedEntries);

    default:
    {
        FEVFileExchangeResultInfo ResultInfo;
        ResultInfo.Result = EEVFileExchangeResult::StorageValidationFailed;
        ResultInfo.UserMessage = TEXT("The requested import operation is not supported.");
        ResultInfo.DebugMessage = TEXT("ExecuteImportDatabaseOperation received an unknown import operation.");

        return ResultInfo;
    }
    }
}

void UEVGameInstance::PopulateImportResultFileInfo(FEVFileExchangeResultInfo& ResultInfo,
                                                   const FEVFileExchangeResultInfo& PickResult, int32 ByteCount) const
{
    ResultInfo.FileName = PickResult.FileName;

    ResultInfo.ByteSize = ByteCount;
}

FEVRequestedActionInfo UEVGameInstance::HandleDownloadTemplateRequested(const FEVFileOperationInfo& FileOperationInfo)
{
    FEVRequestedActionInfo ActionInfo;
    ActionInfo.Source = EEVRequestedActionSource::ImportExport;
    ActionInfo.Type = EEVRequestedActionType::DownloadDBTemplate;

    if (!VocabularyStorageService)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("Vocabulary storage service is not available."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("HandleDownloadTemplateRequested: VocabularyStorageService is null."));

        return ActionInfo;
    }

    if (!DeviceService)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("Device service is not available."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("HandleDownloadTemplateRequested: DeviceService is null."));

        return ActionInfo;
    }

    TArray<uint8> TemplateBytes;

    const FEVFileExchangeResultInfo TemplateGenerationResult =
        VocabularyStorageService->GenerateDatabaseExportTemplate(FileOperationInfo.FileExtensionType, TemplateBytes);

    if (!TemplateGenerationResult.IsSuccess())
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TemplateGenerationResult.UserMessage);
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("Failed to generate database template: %s"),
               *TemplateGenerationResult.DebugMessage);

        return ActionInfo;
    }

    if (TemplateBytes.IsEmpty())
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("The generated database template is empty."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("GenerateDatabaseExportTemplate succeeded but returned an empty buffer."));

        return ActionInfo;
    }

    PendingFileSavePurpose = EEVPendingFileSavePurpose::DownloadTemplate;

    DeviceService->SaveBytesToUserSelectedLocation(
        FileOperationInfo.FileExtensionType,
        FEVFileExchangeDefaults::GetTemplateFileName(FileOperationInfo.FileExtensionType), TemplateBytes);

    ActionInfo.Status = EEVRequestedActionStatus::InProgress;
    ActionInfo.Message = FText::FromString(TEXT("Preparing template download..."));
    ActionInfo.GenerateColor();

    return ActionInfo;
}

FEVRequestedActionInfo UEVGameInstance::HandleExportDBRequested(const FEVFileOperationInfo& FileOperationInfo)
{
    FEVRequestedActionInfo ActionInfo;
    ActionInfo.Source = EEVRequestedActionSource::ImportExport;
    ActionInfo.Type = EEVRequestedActionType::ExportDB;

    if (!VocabularyStorageService)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("Vocabulary storage service is not available."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("HandleExportDBRequested: VocabularyStorageService is null."));

        return ActionInfo;
    }

    if (!DeviceService)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("Device service is not available."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("HandleExportDBRequested: DeviceService is null."));

        return ActionInfo;
    }

    TArray<uint8> ExportBytes;

    const FEVFileExchangeResultInfo ExportGenerationResult =
        VocabularyStorageService->GenerateDatabaseExport(FileOperationInfo.FileExtensionType, ExportBytes);

    if (!ExportGenerationResult.IsSuccess())
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(ExportGenerationResult.UserMessage);
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("Failed to generate database export: %s"), *ExportGenerationResult.DebugMessage);

        return ActionInfo;
    }

    if (ExportBytes.IsEmpty())
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("The generated database export is empty."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("GenerateDatabaseExport succeeded but returned an empty buffer."));

        return ActionInfo;
    }

    PendingFileSavePurpose = EEVPendingFileSavePurpose::ExportDatabase;

    DeviceService->SaveBytesToUserSelectedLocation(
        FileOperationInfo.FileExtensionType,
        FEVFileExchangeDefaults::GetDatabaseExportFileName(FileOperationInfo.FileExtensionType), ExportBytes);

    ActionInfo.Status = EEVRequestedActionStatus::InProgress;
    ActionInfo.Message = FText::FromString(TEXT("Preparing database export..."));
    ActionInfo.GenerateColor();

    return ActionInfo;
}

FEVRequestedActionInfo UEVGameInstance::HandleImportDBOverwriteRequested(const FEVFileOperationInfo& FileOperationInfo)
{
    FEVRequestedActionInfo ActionInfo;
    ActionInfo.Source = EEVRequestedActionSource::ImportExport;
    ActionInfo.Type = EEVRequestedActionType::ImportDBOverwrite;

    if (!DeviceService)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("Device service is not available."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("HandleImportDBOverwriteRequested: DeviceService is null."));

        return ActionInfo;
    }

    if (FileOperationInfo.FileExtensionType == EEVFileExtensionType::Unknown)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("No import file type was selected."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("HandleImportDBOverwriteRequested received Unknown extension."));

        return ActionInfo;
    }

    PendingImportFileOperationInfo = FileOperationInfo;

    DeviceService->PickImportFile(FileOperationInfo.FileExtensionType);

    ActionInfo.Status = EEVRequestedActionStatus::InProgress;
    ActionInfo.Message = FText::FromString(TEXT("Select a database file to import."));
    ActionInfo.GenerateColor();

    return ActionInfo;
}

FEVRequestedActionInfo UEVGameInstance::HandleImportDBAppendRequested(const FEVFileOperationInfo& FileOperationInfo)
{
    FEVRequestedActionInfo ActionInfo;
    ActionInfo.Source = EEVRequestedActionSource::ImportExport;
    ActionInfo.Type = EEVRequestedActionType::ImportDBAppend;

    if (!DeviceService)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("Device service is not available."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("HandleImportDBAppendRequested: DeviceService is null."));

        return ActionInfo;
    }

    if (FileOperationInfo.FileExtensionType == EEVFileExtensionType::Unknown)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("No import file type was selected."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("HandleImportDBAppendRequested received Unknown extension."));

        return ActionInfo;
    }

    PendingImportFileOperationInfo = FileOperationInfo;

    DeviceService->PickImportFile(FileOperationInfo.FileExtensionType);

    ActionInfo.Status = EEVRequestedActionStatus::InProgress;
    ActionInfo.Message = FText::FromString(TEXT("Select a database file to append."));
    ActionInfo.GenerateColor();

    return ActionInfo;
}

FEVFileOperationCompletedFromGameInstance& UEVGameInstance::OnFileOperationCompleted()
{
    return FileOperationCompletedDelegate;
}

FEVRequestedActionInfo
UEVGameInstance::ConvertFileExchangeResultToRequestedAction(const FEVFileExchangeResultInfo& ResultInfo) const
{
    FEVRequestedActionInfo ActionInfo;
    ActionInfo.Source = EEVRequestedActionSource::ImportExport;

    ActionInfo.Status = ResultInfo.IsSuccess() ? EEVRequestedActionStatus::Completed : EEVRequestedActionStatus::Failed;

    ActionInfo.Message = FText::FromString(ResultInfo.UserMessage);

    ActionInfo.GenerateColor();

    return ActionInfo;
}

bool UEVGameInstance::GetStoredNotificationSettings(FEVPopUpSettingsInfo& OutSettings) const
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot read stored notification settings: DeviceService is nullptr."));
        return false;
    }

    bool bEnabled = false;
    int32 IntervalSeconds = 0;
    int32 NotificationModeValue = 0;

    if (!DeviceService->GetStoredVocabularyNotificationSettings(bEnabled, IntervalSeconds, NotificationModeValue))
    {
        return false;
    }

    OutSettings = FEVPopUpSettingsInfo();

    if (bEnabled)
    {
        bool bFoundInterval = false;
        for (const EEVPopUpIntervals Interval : FEVPopUpSettingsInfo::GetAllIntervals())
        {
            if (FEVPopUpSettingsInfo::GetIntervalSeconds(Interval) == IntervalSeconds)
            {
                OutSettings.PopUpIntervals = Interval;
                bFoundInterval = true;
                break;
            }
        }

        if (!bFoundInterval)
        {
            UE_LOG(LogTemp, Warning, TEXT("Stored notification interval %d seconds is unsupported."), IntervalSeconds);
            OutSettings.PopUpIntervals = EEVPopUpIntervals::TurnedOff;
        }
    }

    bool bFoundMode = false;
    for (const EEVNotificationMode Mode : FEVPopUpSettingsInfo::GetAllNotificationModes())
    {
        if (static_cast<int32>(Mode) == NotificationModeValue)
        {
            OutSettings.NotificationMode = Mode;
            bFoundMode = true;
            break;
        }
    }

    if (!bFoundMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("Stored notification mode %d is unsupported."), NotificationModeValue);
    }

    return true;
}

bool UEVGameInstance::ConsumePendingNotificationWord(FString& OutWord) const
{
    OutWord.Empty();
    return DeviceService && DeviceService->ConsumePendingNotificationWord(OutWord);
}

bool UEVGameInstance::AreNotificationsEnabled() const
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot check notifications: DeviceService is nullptr."));

        return false;
    }

    return DeviceService->AreNotificationsEnabled();
}

bool UEVGameInstance::HasRequestedNotificationPermission() const
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot check notification permission history: DeviceService is nullptr."));

        return false;
    }

    return DeviceService->HasRequestedNotificationPermission();
}

bool UEVGameInstance::RequestNotificationPermission()
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot request notification permission: DeviceService is nullptr."));

        return false;
    }

    return DeviceService->RequestNotificationPermission();
}

void UEVGameInstance::OpenNotificationSettings()
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot open notification settings: DeviceService is nullptr."));

        return;
    }

    DeviceService->OpenNotificationSettings();
}

void UEVGameInstance::TestDeviceAlarm()
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot open notification settings: DeviceService is nullptr."));

        return;
    }

    DeviceService->TestAlarm();
}

void UEVGameInstance::HandleNotificationPermissionResult(const bool bGranted)
{
    NotificationPermissionResultDelegate.Broadcast(bGranted);
}