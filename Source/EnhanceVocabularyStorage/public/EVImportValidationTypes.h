#pragma once

#include "CoreMinimal.h"

enum class EEVValidationProblemScope : uint8
{
    File,
    Header,
    Row,
    Database
};

struct FEVValidationFailedEntry
{
    EEVValidationProblemScope Scope = EEVValidationProblemScope::Row;

    int32 RowNumber = INDEX_NONE;

    FString ColumnName;

    FString Entry;

    FString Reason;
};