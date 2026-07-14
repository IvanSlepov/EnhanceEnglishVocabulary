#pragma once

#include "CoreMinimal.h"

enum class EEVDatabaseColumnRole : uint8
{
    Internal,
    ImportExport
};

struct FEVDatabaseColumnDefinition
{
    FString Name;
    FString SqlDefinition;
    EEVDatabaseColumnRole Role = EEVDatabaseColumnRole::Internal;
};

class ENHANCEVOCABULARYSTORAGE_API FEVVocabularyDatabaseSchema
{
public:
    static const FString& GetTableName();

    static const TArray<FEVDatabaseColumnDefinition>& GetColumns();

    static TArray<FEVDatabaseColumnDefinition> GetImportExportColumns();

    static TArray<FString> GetImportExportColumnNames();
};