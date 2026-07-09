// Fill out your copyright notice in the Description page of Project Settings.

#include "EVGameInstance.h"

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
        // Creating and HttpService instance within
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
        ConnectivityService->OnConnectionStateChanged.AddDynamic(this, &UEVGameInstance::HandleConnectionStateChanged);

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
        break;
    case EEVWordLookupResult::Exists:
        return EEVVocabularyStorageServiceResult::WordExists;
        break;
    case EEVWordLookupResult::DoesNotExist:
        return EEVVocabularyStorageServiceResult::WordDoesNotExist;
        break;
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
    Entry.Definition = WordSearchResult.Definition;
    Entry.Usage = WordSearchResult.Usage;
    Entry.TranslationRu = WordSearchResult.TranslationRu;
    Entry.TranslationUa = WordSearchResult.TranslationUa;

    if (VocabularyStorageService->SaveVocabularyEntry(Entry))
    {
        return true;
    }

    return false;
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

bool UEVGameInstance::GetVocabularyEntries(TArray<FVocabularyEntry>& OutVocabularyEntries, int32 EntryNumber)
{
    if (VocabularyStorageService)
    {
        OutVocabularyEntries = VocabularyStorageService->GetVocabularyEntries(EntryNumber);
        return true;
    }

    UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));
    return false;
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
        FWordSearchResult Result;
        Result.bSuccess = false;
        Result.ErrorMessage = TEXT("WordSearchService/Real Online Search is null");
        // return Result;
    }

    WordSearchService->SearchWordOnline(Word, DefinitionUsageProvider, TranslationProvider);
}

FEVRequestedActionInfo UEVGameInstance::HandleFileOperationRequested(const FEVFileOperationInfo& FileOperationInfo)
{
    switch (FileOperationInfo.OperationType)
    {
    case EEVFileOperationType::DownloadTemplate:
        return HandleDownloadTemplateRequested(FileOperationInfo);

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
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ConnectivityService is null in UEVGameInstance::GetConnectionState()"));
        return EEVConnectionState::Offline;
    }
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

void UEVGameInstance::HandleFileSaved(const FEVFileExchangeResultInfo& ResultInfo)
{
    UE_LOG(LogTemp, Log, TEXT("File saved result: %d | File: %s | Bytes: %lld | Message: %s | Debug: %s"),
           static_cast<uint8>(ResultInfo.Result), *ResultInfo.FileName, ResultInfo.ByteSize, *ResultInfo.UserMessage,
           *ResultInfo.DebugMessage);

    const FEVRequestedActionInfo ActionInfo = ConvertFileExchangeResultToRequestedAction(ResultInfo);

    FileOperationCompletedDelegate.Broadcast(ActionInfo);
}

void UEVGameInstance::HandleImportFilePicked(const FEVFileExchangeResultInfo& ResultInfo, const TArray<uint8>& Bytes)
{
    UE_LOG(LogTemp, Log,
           TEXT("Import file picked result: %d | File: %s | Bytes: %lld | ReceivedBytes: %d | Message: %s | Debug: %s"),
           static_cast<uint8>(ResultInfo.Result), *ResultInfo.FileName, ResultInfo.ByteSize, Bytes.Num(),
           *ResultInfo.UserMessage, *ResultInfo.DebugMessage);

    // Import logic comes later:
    // Result success -> pass Bytes to Storage
    // Storage validates CSV/data
    // Append/Overwrite based on pending operation.
}

FEVRequestedActionInfo UEVGameInstance::HandleDownloadTemplateRequested(const FEVFileOperationInfo& FileOperationInfo)
{
    FEVRequestedActionInfo ActionInfo;
    ActionInfo.Source = EEVRequestedActionSource::ImportExport;

    if (!VocabularyStorageService)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("Vocabulary storage service is not available."));
        ActionInfo.GenerateColor();
        return ActionInfo;
    }

    if (!DeviceService)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("Device service is not available."));
        ActionInfo.GenerateColor();
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
        return ActionInfo;
    }

    DeviceService->SaveBytesToUserSelectedLocation(FileOperationInfo.FileExtensionType,
                                                   TEXT("VocabularyDB_Template.csv"), TemplateBytes);

    ActionInfo.Status = EEVRequestedActionStatus::InProgress;
    ActionInfo.Message = FText::FromString(TEXT("Preparing template download..."));
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