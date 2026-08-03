#pragma once

#include "CoreMinimal.h"
#include "EVVocabularyTypes.h"

enum class EEVDatabaseColumnRole : uint8
{
    Internal,
    ImportExport
};

enum class EEVImportRequirement : uint8
{
    None,
    Required,
    Optional
};

/**
 * Central description of a vocabulary field that is persisted in SQLite.
 *
 * Adding a normal FString field now requires two adjacent changes in the Core module:
 * 1. add the UPROPERTY to FVocabularyEntry;
 * 2. add its descriptor to FEVVocabularyFieldRegistry::GetPersistentFields().
 *
 * SQL generation, statement binding, row reading and CSV import/export all consume
 * this registry, so they no longer maintain their own field order.
 */
struct ENHANCEVOCABULARYCORE_API FEVDatabaseColumnDefinition
{
    FString Name;
    FString SqlDefinition;
    EEVDatabaseColumnRole Role = EEVDatabaseColumnRole::Internal;
    EEVImportRequirement ImportRequirement = EEVImportRequirement::None;
    FString ImportGuidance;

    // Null for database-only columns such as Id, CreatedAt and UpdatedAt.
    FString FVocabularyEntry::*EntryMember = nullptr;

    // Word identifies a row and is intentionally not included in UPDATE SET lists.
    bool bEditable = true;

    bool IsEntryField() const
    {
        return EntryMember != nullptr;
    }
};

class ENHANCEVOCABULARYCORE_API FEVVocabularyFieldRegistry
{
public:
    static const TArray<FEVDatabaseColumnDefinition>& GetPersistentFields();
    static TArray<FEVDatabaseColumnDefinition> GetEditablePersistentFields();
    static TArray<FString> GetPersistentColumnNames();
};
