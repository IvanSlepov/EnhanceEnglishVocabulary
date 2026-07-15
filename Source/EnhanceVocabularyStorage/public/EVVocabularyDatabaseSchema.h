#pragma once

#include "CoreMinimal.h"

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

struct FEVDatabaseColumnDefinition
{
    FString Name;
    FString SqlDefinition;
    EEVDatabaseColumnRole Role;

    EEVImportRequirement ImportRequirement;
    FString ImportGuidance;
};

class ENHANCEVOCABULARYSTORAGE_API FEVVocabularyDatabaseSchema
{
public:
    static const FString& GetTableName();

    static const TArray<FEVDatabaseColumnDefinition>& GetColumns();

    static TArray<FEVDatabaseColumnDefinition> GetImportExportColumns();

    static TArray<FString> GetImportExportColumnNames();
};