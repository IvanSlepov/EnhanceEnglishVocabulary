#include "EVImportValidationReportFormatter.h"

FString FEVImportValidationReportFormatter::BuildCsvReport(const TArray<FEVDatabaseColumnDefinition>& Columns,
                                                           const TArray<FEVImportValidationRule>& Rules,
                                                           const TArray<FEVValidationFailedEntry>& Problems)
{
    FString CsvContent;

    CsvContent += TEXT("IMPORT RULES\r\n");
    CsvContent += TEXT("RuleType,Subject,Requirement,Guidance\r\n");

    for (const FEVDatabaseColumnDefinition& Column : Columns)
    {
        CsvContent +=
            FString::Printf(TEXT("%s,%s,%s,%s\r\n"), *EscapeCsvField(TEXT("Column")), *EscapeCsvField(Column.Name),
                            *EscapeCsvField(ColumnRequirementToString(Column.ImportRequirement)),
                            *EscapeCsvField(Column.ImportGuidance));
    }

    for (const FEVImportValidationRule& Rule : Rules)
    {
        CsvContent += FString::Printf(TEXT("%s,%s,%s,%s\r\n"),
                                      *EscapeCsvField(FEVImportValidationRules::ScopeToString(Rule.Scope)),
                                      *EscapeCsvField(Rule.Subject),
                                      *EscapeCsvField(FEVImportValidationRules::RequirementToString(Rule.Requirement)),
                                      *EscapeCsvField(Rule.Guidance));
    }

    CsvContent += TEXT("\r\n");

    CsvContent += TEXT("VALIDATION PROBLEMS\r\n");
    CsvContent += TEXT("Scope,RowNumber,ColumnName,ProblematicEntry,Reason\r\n");

    for (const FEVValidationFailedEntry& Problem : Problems)
    {
        CsvContent += FString::Printf(TEXT("%s,%d,%s,%s,%s\r\n"), *EscapeCsvField(ProblemScopeToString(Problem.Scope)),
                                      Problem.RowNumber, *EscapeCsvField(Problem.ColumnName),
                                      *EscapeCsvField(Problem.Entry), *EscapeCsvField(Problem.Reason));
    }

    return CsvContent;
}

FString FEVImportValidationReportFormatter::EscapeCsvField(const FString& Value)
{
    FString EscapedValue = Value;

    EscapedValue.ReplaceInline(TEXT("\""), TEXT("\"\""));

    return FString::Printf(TEXT("\"%s\""), *EscapedValue);
}

FString FEVImportValidationReportFormatter::ProblemScopeToString(EEVValidationProblemScope Scope)
{
    switch (Scope)
    {
    case EEVValidationProblemScope::File:
        return TEXT("File");

    case EEVValidationProblemScope::Header:
        return TEXT("Header");

    case EEVValidationProblemScope::Row:
        return TEXT("Row");

    case EEVValidationProblemScope::Database:
        return TEXT("Database");

    default:
        return TEXT("Unknown");
    }
}

FString FEVImportValidationReportFormatter::ColumnRequirementToString(EEVImportRequirement Requirement)
{
    switch (Requirement)
    {
    case EEVImportRequirement::Required:
        return TEXT("Required");

    case EEVImportRequirement::Optional:
        return TEXT("Optional");

    case EEVImportRequirement::None:
        return TEXT("Not applicable");

    default:
        return TEXT("Unknown");
    }
}