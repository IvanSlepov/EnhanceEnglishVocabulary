#pragma once

#include "CoreMinimal.h"

#include "EVImportValidationRules.h"
#include "EVVocabularyDatabaseSchema.h"
#include "EVImportValidationTypes.h"

class ENHANCEVOCABULARYSTORAGE_API FEVImportValidationReportFormatter
{
public:
    static FString BuildCsvReport(const TArray<FEVDatabaseColumnDefinition>& Columns,
                                  const TArray<FEVImportValidationRule>& Rules,
                                  const TArray<FEVValidationFailedEntry>& Problems);

private:
    static FString EscapeCsvField(const FString& Value);

    static FString ProblemScopeToString(EEVValidationProblemScope Scope);

    static FString ColumnRequirementToString(EEVImportRequirement Requirement);
};