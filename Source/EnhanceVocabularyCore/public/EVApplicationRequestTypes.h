#pragma once

#include "CoreMinimal.h"
#include "EVVocabularyFilterTypes.h"
#include "EVVocabularyLanguageTypes.h"
#include "EVVocabularyTypes.h"

enum class EEVApplicationConnectivityState : uint8
{
    Unknown,
    Offline,
    Connecting,
    Online
};

enum class EEVApplicationOperationResult : uint8
{
    Succeeded,
    Failed,
    Cancelled,
    Busy,
    Unavailable
};

enum class EEVVocabularyMutationType : uint8
{
    Save,
    Update,
    Delete
};

enum class EEVVocabularyChangeType : uint8
{
    Added,
    Updated,
    Removed,
    BulkChanged,
    Reset
};

struct ENHANCEVOCABULARYCORE_API FEVVocabularySearchRequest
{
    FGuid RequestId;
    FString Word;
    FName DefinitionProviderId;
    FName TranslationProviderId;
};

struct ENHANCEVOCABULARYCORE_API FEVVocabularySearchOutcome
{
    FGuid RequestId;
    EEVApplicationOperationResult Result = EEVApplicationOperationResult::Failed;
    FEVVocabularyRecord Record;
    FText Message;
};

struct ENHANCEVOCABULARYCORE_API FEVVocabularyRecordRequest
{
    FGuid RequestId;
    FString Word;
};

struct ENHANCEVOCABULARYCORE_API FEVVocabularyRecordOutcome
{
    FGuid RequestId;
    EEVApplicationOperationResult Result = EEVApplicationOperationResult::Failed;
    bool bExists = false;
    FEVVocabularyRecord Record;
    FText Message;
};

struct ENHANCEVOCABULARYCORE_API FEVVocabularyQueryRequest
{
    FGuid RequestId;
    FString SearchPrefix;
    FEVVocabularyQueryCriteria Criteria;
    int32 Limit = 0;
    int32 Offset = 0;
};

struct ENHANCEVOCABULARYCORE_API FEVVocabularyQueryOutcome
{
    FGuid RequestId;
    EEVApplicationOperationResult Result = EEVApplicationOperationResult::Failed;
    TArray<FEVVocabularyRecord> Records;
    int32 TotalEntries = 0;
    FText Message;
};

struct ENHANCEVOCABULARYCORE_API FEVVocabularyMutationRequest
{
    FGuid RequestId;
    EEVVocabularyMutationType MutationType = EEVVocabularyMutationType::Save;
    FEVVocabularyRecord Record;
};

struct ENHANCEVOCABULARYCORE_API FEVVocabularyMutationOutcome
{
    FGuid RequestId;
    EEVVocabularyMutationType MutationType = EEVVocabularyMutationType::Save;
    EEVApplicationOperationResult Result = EEVApplicationOperationResult::Failed;
    FEVVocabularyRecord Record;
    FText Message;
};

struct ENHANCEVOCABULARYCORE_API FEVVocabularyChangeInfo
{
    FGuid ChangeId;
    FGuid CorrelationId;
    FName OriginId;
    EEVVocabularyChangeType ChangeType = EEVVocabularyChangeType::BulkChanged;
    TArray<FString> AffectedNormalizedWords;
    bool bFullRefreshRequired = false;
};

struct ENHANCEVOCABULARYCORE_API FEVVocabularyPreferencesChangeRequest
{
    FGuid RequestId;
    FEVVocabularyLanguagePreferences Preferences;
};

struct ENHANCEVOCABULARYCORE_API FEVVocabularyPreferencesState
{
    FGuid RequestId;
    EEVApplicationOperationResult Result = EEVApplicationOperationResult::Succeeded;
    FEVVocabularyLanguagePreferences Preferences;
    FText Message;
};
