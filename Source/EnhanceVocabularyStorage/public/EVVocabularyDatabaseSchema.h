#pragma once

#include "CoreMinimal.h"
#include "EVVocabularyFieldRegistry.h"

class ENHANCEVOCABULARYSTORAGE_API FEVVocabularyDatabaseSchema
{
public:
    static const FString& GetTableName();

    static const TArray<FEVDatabaseColumnDefinition>& GetColumns();

    static TArray<FEVDatabaseColumnDefinition> GetImportExportColumns();

    static TArray<FString> GetImportExportColumnNames();
};
