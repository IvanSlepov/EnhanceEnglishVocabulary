#pragma once

#include "CoreMinimal.h"
#include "EVVocabularyTypes.h"
#include "EVWordEntryActionTypes.generated.h"

UENUM(BlueprintType)
enum class EEVWordEntryActionType : uint8
{
    Unknown,
    ViewEntry,
    CloseEntry,
    EditEntry,
    SaveEditedEntry,
    DeleteEntry,
    ConfirmDeleteEntry,
    CancelDeleteEntry
};

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYCORE_API FEVWordEntryActionInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    EEVWordEntryActionType ActionType = EEVWordEntryActionType::Unknown;

    UPROPERTY(BlueprintReadWrite)
    FVocabularyEntry EntryInfo;
};