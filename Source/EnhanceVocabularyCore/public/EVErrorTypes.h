#pragma once

#include "EVErrorTypes.generated.h"

UENUM(BlueprintType)
enum class EEVErrorSource : uint8
{
    Unknown,
    Database,
    AddWord,
    ReviewWords,
    ImportExport,
    Settings,
    ConnectionModule
};

UENUM(BlueprintType)
enum class EEVErrorType : uint8
{
    Unknown,
    EmptyString,
    InvalidInput,
    DuplicateWord,
    SearchError,
    DatabaseError,
    InternalError,
    ConnectionError
};

UENUM(BlueprintType)
enum class EEVInputValidationResult : uint8
{
    Valid,
    EmptyInput,
    InvalidCharacters
};

USTRUCT(BlueprintType)
struct FEVErrorInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EEVErrorSource Source = EEVErrorSource::Unknown;

    UPROPERTY(BlueprintReadOnly)
    EEVErrorType Type = EEVErrorType::Unknown;

    UPROPERTY(BlueprintReadOnly)
    FText Message;
};