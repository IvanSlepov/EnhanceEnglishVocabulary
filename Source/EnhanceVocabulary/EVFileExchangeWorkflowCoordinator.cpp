#include "EVFileExchangeWorkflowCoordinator.h"

#include "EVGameInstance.h"
#include "EVVocabularyLibraryApplicationPort.h"
#include "EVWidgetCommonEvents.h"

void UEVFileExchangeWorkflowCoordinator::Initialize(UEVGameInstance* InGameInstance,
                                                    IEVVocabularyLibraryApplicationPort* InLibraryPort,
                                                    IEVWidgetCommonEvents* InLegacyEvents)
{
    GameInstance = InGameInstance;
    UpdatePresentationPorts(InLibraryPort, InLegacyEvents);
}

void UEVFileExchangeWorkflowCoordinator::UpdatePresentationPorts(IEVVocabularyLibraryApplicationPort* InLibraryPort,
                                                                 IEVWidgetCommonEvents* InLegacyEvents)
{
    LibraryPort = InLibraryPort;
    LegacyEvents = InLegacyEvents;
}

void UEVFileExchangeWorkflowCoordinator::RequestOperation(const FEVFileOperationInfo& Operation)
{
    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot request a file operation: GameInstance is unavailable."));
        return;
    }

    if (Operation.OperationType == EEVFileOperationType::ImportDBOverwrite ||
        Operation.OperationType == EEVFileOperationType::ImportDBAppend)
    {
        PendingOperation = Operation;
        ConfirmationRequested.Broadcast(Operation.OperationType == EEVFileOperationType::ImportDBOverwrite
                                            ? EEVConfirmationDialogType::OverwriteDB
                                            : EEVConfirmationDialogType::AppendDB,
                                        EEVWordEntryActionType::Unknown);
        return;
    }

    LoadingChanged.Broadcast(true);
    const FEVRequestedActionInfo Result = GameInstance->HandleFileOperationRequested(Operation);
    if (Result.Status != EEVRequestedActionStatus::InProgress)
    {
        LoadingChanged.Broadcast(false);
        StatusRequested.Broadcast(Result);
    }
}

void UEVFileExchangeWorkflowCoordinator::HandleOperationCompleted(const FEVRequestedActionInfo& Result)
{
    LoadingChanged.Broadcast(false);
    StatusRequested.Broadcast(Result);
}

void UEVFileExchangeWorkflowCoordinator::HandleImportFilePickCompleted(const FEVFileExchangeResultInfo& Result)
{
    LoadingChanged.Broadcast(false);

    if (Result.Result == EEVFileExchangeResult::CancelledByUser)
    {
        UE_LOG(LogTemp, Warning, TEXT("Import file selection cancelled by user."));
        PendingOperation = FEVFileOperationInfo();
        return;
    }

    FEVRequestedActionInfo ActionInfo;
    ActionInfo.Source = EEVRequestedActionSource::ImportExport;
    ActionInfo.Type = EEVRequestedActionType::ImportDBOverwrite;
    ActionInfo.Status = Result.IsSuccess() ? EEVRequestedActionStatus::Completed : EEVRequestedActionStatus::Failed;
    ActionInfo.Message = FText::FromString(Result.UserMessage);
    ActionInfo.GenerateColor();
    StatusRequested.Broadcast(ActionInfo);
    PendingOperation = FEVFileOperationInfo();

    if (!Result.IsSuccess())
    {
        return;
    }

    if (LibraryPort)
    {
        FEVVocabularyChangeInfo ChangeInfo;
        ChangeInfo.ChangeId = FGuid::NewGuid();
        ChangeInfo.OriginId = FName(TEXT("FileExchange"));
        ChangeInfo.ChangeType = EEVVocabularyChangeType::BulkChanged;
        ChangeInfo.bFullRefreshRequired = true;
        LibraryPort->ApplyVocabularyChanged(ChangeInfo);
    }
    else if (LegacyEvents)
    {
        LegacyEvents->HandleReviewWordsRefresh();
    }
}

bool UEVFileExchangeWorkflowCoordinator::ResolveConfirmation(const EEVConfirmationDialogType DialogType,
                                                             const bool bConfirmed)
{
    const bool bOverwrite = DialogType == EEVConfirmationDialogType::OverwriteDB;
    const bool bAppend = DialogType == EEVConfirmationDialogType::AppendDB;
    if (!bOverwrite && !bAppend)
    {
        return false;
    }

    const EEVFileOperationType ExpectedType =
        bOverwrite ? EEVFileOperationType::ImportDBOverwrite : EEVFileOperationType::ImportDBAppend;
    if (!bConfirmed)
    {
        UE_LOG(LogTemp, Warning, TEXT("Import database operation cancelled by user."));
        PendingOperation = FEVFileOperationInfo();
        return true;
    }

    if (PendingOperation.OperationType != ExpectedType)
    {
        UE_LOG(LogTemp, Error, TEXT("Import confirmation received without the matching pending operation."));
        PendingOperation = FEVFileOperationInfo();
        return true;
    }

    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot start import: GameInstance is unavailable."));
        PendingOperation = FEVFileOperationInfo();
        return true;
    }

    LoadingChanged.Broadcast(true);
    const FEVRequestedActionInfo Result = GameInstance->HandleFileOperationRequested(PendingOperation);
    if (Result.Status != EEVRequestedActionStatus::InProgress)
    {
        LoadingChanged.Broadcast(false);
        StatusRequested.Broadcast(Result);
        PendingOperation = FEVFileOperationInfo();
    }
    return true;
}
