#include "EVImportValidationRules.h"

const TArray<FEVImportValidationRule>& FEVImportValidationRules::GetGeneralRules()
{
    static const TArray<FEVImportValidationRule> Rules = {
        {EEVImportValidationRuleScope::File, TEXT("Header"), EEVImportValidationRuleRequirement::Required,
         TEXT("The CSV must contain all expected columns in the expected order.")},
        {EEVImportValidationRuleScope::File, TEXT("Rows"), EEVImportValidationRuleRequirement::Required,
         TEXT("Each row must not contain more values than the configured import columns.")},
        {EEVImportValidationRuleScope::File, TEXT("Duplicates"), EEVImportValidationRuleRequirement::NotAllowed,
         TEXT("Duplicate normalized words inside the imported file are not allowed.")},
        {EEVImportValidationRuleScope::Append, TEXT("Database duplicates"),
         EEVImportValidationRuleRequirement::NotAllowed,
         TEXT("During Append, imported words must not already exist in the current database.")}};

    return Rules;
}

FString FEVImportValidationRules::ScopeToString(EEVImportValidationRuleScope Scope)
{
    switch (Scope)
    {
    case EEVImportValidationRuleScope::File:
        return TEXT("File");

    case EEVImportValidationRuleScope::Column:
        return TEXT("Column");

    case EEVImportValidationRuleScope::Append:
        return TEXT("Append");

    default:
        return TEXT("Unknown");
    }
}

FString FEVImportValidationRules::RequirementToString(EEVImportValidationRuleRequirement Requirement)
{
    switch (Requirement)
    {
    case EEVImportValidationRuleRequirement::Required:
        return TEXT("Required");

    case EEVImportValidationRuleRequirement::Optional:
        return TEXT("Optional");

    case EEVImportValidationRuleRequirement::NotAllowed:
        return TEXT("Not allowed");

    case EEVImportValidationRuleRequirement::NotApplicable:
        return TEXT("Not applicable");

    default:
        return TEXT("Unknown");
    }
}