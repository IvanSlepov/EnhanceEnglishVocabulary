#pragma once

#include "CoreMinimal.h"
#include "EVFileExchangeTypes.generated.h"

UENUM(BlueprintType)
enum class EEVFileOperationType : uint8
{
    Unknown,

    ImportDBOverwrite UMETA(DisplayName = "Import DB - Overwrite"),
    ImportDBAppend UMETA(DisplayName = "Import DB - Append"),
    ExportDB UMETA(DisplayName = "Export DB"),
    DownloadTemplate UMETA(DisplayName = "Download Template")
};

UENUM(BlueprintType)
enum class EEVFileExtensionType : uint8
{
    Unknown,
    Csv UMETA(DisplayName = ".csv"),
    Db UMETA(DisplayName = ".db"),
    Txt UMETA(DisplayName = ".txt")
};

UENUM(BlueprintType)
enum class EEVFileExchangeResult : uint8
{
    Success,
    CancelledByUser,
    PermissionDenied,
    InvalidExtension,
    EmptyFile,
    InvalidStructure,
    ReadFailed,
    WriteFailed,
    UnsupportedPlatform,
    StorageValidationFailed,
    DatabaseTransactionFailed,
    UnknownError
};

USTRUCT(BlueprintType)
struct FEVFileOperationInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EEVFileOperationType OperationType = EEVFileOperationType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EEVFileExtensionType FileExtensionType = EEVFileExtensionType::Unknown;
};

USTRUCT(BlueprintType)
struct FEVFileExchangeResultInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EEVFileExchangeResult Result = EEVFileExchangeResult::UnknownError;

    UPROPERTY(BlueprintReadOnly)
    FString UserMessage;

    UPROPERTY(BlueprintReadOnly)
    FString DebugMessage;

    UPROPERTY(BlueprintReadOnly)
    FString FileName;

    UPROPERTY(BlueprintReadOnly)
    int64 ByteSize = 0;

    bool IsSuccess() const
    {
        return Result == EEVFileExchangeResult::Success;
    }
};