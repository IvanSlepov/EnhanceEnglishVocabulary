// Fill out your copyright notice in the Description page of Project Settings.

#include "EVWordInputValidator.h"

FString FEVWordInputValidator::NormalizeWordInput(const FString& RawInput)
{
    FString Result = RawInput.TrimStartAndEnd();

    while (Result.Contains(TEXT("  ")))
    {
        Result = Result.Replace(TEXT("  "), TEXT(" "));
    }

    return Result.ToLower();
}

bool FEVWordInputValidator::ContainsOnlyLettersAndSpaces(const FString& Input)
{
    for (const TCHAR Char : Input)
    {
        if (!FChar::IsAlpha(Char) && !FChar::IsWhitespace(Char))
        {
            return false;
        }
    }

    return true;
}

EEVInputValidationResult FEVWordInputValidator::ValidateSearchInput(const FString& RawInput, FString& OutNormalizedWord,
                                                                    FText& OutErrorMessage)
{
    OutNormalizedWord = NormalizeWordInput(RawInput);
    OutErrorMessage = FText::GetEmpty();

    if (OutNormalizedWord.IsEmpty())
    {
        OutErrorMessage = FText::FromString(TEXT("Please enter a word."));
        return EEVInputValidationResult::EmptyInput;
    }

    if (!ContainsOnlyLettersAndSpaces(OutNormalizedWord))
    {
        OutErrorMessage = FText::FromString(TEXT("Only letters and spaces are allowed."));
        return EEVInputValidationResult::InvalidCharacters;
    }

    return EEVInputValidationResult::Valid;
}
