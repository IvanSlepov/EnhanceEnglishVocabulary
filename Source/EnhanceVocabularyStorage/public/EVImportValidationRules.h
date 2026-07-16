#pragma once

#include "CoreMinimal.h"

enum class EEVImportValidationRuleScope : uint8
{
    File,
    Column,
    Append
};

enum class EEVImportValidationRuleRequirement : uint8
{
    Required,
    Optional,
    NotAllowed,
    NotApplicable
};

struct FEVImportValidationRule
{
    EEVImportValidationRuleScope Scope = EEVImportValidationRuleScope::File;

    FString Subject;

    EEVImportValidationRuleRequirement Requirement = EEVImportValidationRuleRequirement::NotApplicable;

    FString Guidance;
};

class ENHANCEVOCABULARYSTORAGE_API FEVImportValidationRules
{
public:
    static const TArray<FEVImportValidationRule>& GetGeneralRules();

    static FString ScopeToString(EEVImportValidationRuleScope Scope);

    static FString RequirementToString(EEVImportValidationRuleRequirement Requirement);
};