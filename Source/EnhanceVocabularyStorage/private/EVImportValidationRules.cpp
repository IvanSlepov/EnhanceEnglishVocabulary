#include "EVImportValidationRules.h"

const TArray<FEVImportValidationRule>& FEVImportValidationRules::GetGeneralRules()
{
    static const TArray<FEVImportValidationRule> Rules = {
        {EEVImportValidationRuleScope::File, TEXT("Header"), EEVImportValidationRuleRequirement::Required,
         TEXT("The CSV must contain a Word column. Every other supported column is optional and may appear in any "
              "order.")},
        {EEVImportValidationRuleScope::File, TEXT("Rows"), EEVImportValidationRuleRequirement::Required,
         TEXT("Rows are grouped by normalized Word. Repeated words may provide additional parts of speech, "
              "definitions, translations, synonyms, antonyms, or pronunciation data.")},
        {EEVImportValidationRuleScope::File, TEXT("Optional columns"), EEVImportValidationRuleRequirement::Optional,
         TEXT("A missing optional column is imported as an empty value for every row.")},
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