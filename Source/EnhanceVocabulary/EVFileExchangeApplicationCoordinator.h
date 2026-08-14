#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVFileExchangeDefaults.h"
#include "EVFileExchangeTypes.h"
#include "EVRequestedActionTypes.h"
#include "EVVocabularyTypes.h"
#include "EVFileExchangeApplicationCoordinator.generated.h"

class UEVDeviceService;
class UEVVocabularyStorageService;

DECLARE_MULTICAST_DELEGATE_OneParam(FEVFileOperationCompletedFromGameInstance, const FEVRequestedActionInfo&);
DECLARE_MULTICAST_DELEGATE_OneParam(FEVImportFilePickCompleted, const FEVFileExchangeResultInfo&);

enum class EEVPendingFileSavePurpose : uint8
{
    None,
    DownloadTemplate,
    ExportDatabase,
    ImportValidationReport
};

UCLASS()
class ENHANCEVOCABULARY_API UEVFileExchangeApplicationCoordinator : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UEVVocabularyStorageService* InStorageService, UEVDeviceService* InDeviceService);
    void Shutdown();
    FEVRequestedActionInfo HandleFileOperationRequested(const FEVFileOperationInfo& FileOperationInfo);
    FEVFileOperationCompletedFromGameInstance& OnFileOperationCompleted();
    FEVImportFilePickCompleted& OnImportFilePickCompleted()
    {
        return ImportFilePickCompletedDelegate;
    }

private:
    void HandleFileSaved(const FEVFileExchangeResultInfo& ResultInfo);
    void HandleImportFilePicked(const FEVFileExchangeResultInfo& ResultInfo, const TArray<uint8>& Bytes);
    void CompleteImportFileOperation(const FEVFileExchangeResultInfo& ResultInfo);
    bool TrySaveImportValidationReport(FEVFileExchangeResultInfo ValidationResult,
                                       const TArray<uint8>& ValidationReportBytes);
    FEVFileExchangeResultInfo ExecuteImportDatabaseOperation(const TArray<FEVVocabularyRecord>& ValidatedRecords);
    void PopulateImportResultFileInfo(FEVFileExchangeResultInfo& ResultInfo,
                                      const FEVFileExchangeResultInfo& PickResult, int32 ByteCount) const;
    FEVRequestedActionInfo HandleDownloadTemplateRequested(const FEVFileOperationInfo& FileOperationInfo);
    FEVRequestedActionInfo HandleExportDBRequested(const FEVFileOperationInfo& FileOperationInfo);
    FEVRequestedActionInfo HandleImportDBOverwriteRequested(const FEVFileOperationInfo& FileOperationInfo);
    FEVRequestedActionInfo HandleImportDBAppendRequested(const FEVFileOperationInfo& FileOperationInfo);
    FEVRequestedActionInfo
    ConvertFileExchangeResultToRequestedAction(const FEVFileExchangeResultInfo& ResultInfo) const;

    UPROPERTY(Transient)
    TObjectPtr<UEVVocabularyStorageService> VocabularyStorageService;

    UPROPERTY(Transient)
    TObjectPtr<UEVDeviceService> DeviceService;

    EEVPendingFileSavePurpose PendingFileSavePurpose = EEVPendingFileSavePurpose::None;
    FEVFileOperationInfo PendingImportFileOperationInfo;
    FEVFileExchangeResultInfo PendingImportValidationResult;
    FEVFileOperationCompletedFromGameInstance FileOperationCompletedDelegate;
    FEVImportFilePickCompleted ImportFilePickCompletedDelegate;
};
