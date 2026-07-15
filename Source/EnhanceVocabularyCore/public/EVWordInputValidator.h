// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EVErrorTypes.h"

class ENHANCEVOCABULARYCORE_API FEVWordInputValidator
{
public:
    static EEVInputValidationResult ValidateSearchInput(const FString& RawInput, FString& OutNormalizedWord,
                                                        FText& OutErrorMessage);
    static FString NormalizeWordInput(const FString& RawInput);

    static bool ContainsOnlyLettersSpacesAndInternalApostrophes(const FString& Input);

private:
};
