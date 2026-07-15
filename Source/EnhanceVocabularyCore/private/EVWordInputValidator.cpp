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

bool FEVWordInputValidator::ContainsOnlyLettersSpacesAndInternalApostrophes(const FString& Value)
{
    for (int32 Index = 0; Index < Value.Len(); ++Index)
    {
        const TCHAR Character = Value[Index];

        if (FChar::IsAlpha(Character) || FChar::IsWhitespace(Character))
        {
            continue;
        }

        if (Character == TEXT('\''))
        {
            // Apostrophes cannot begin a word or follow a non-letter.
            if (Index == 0 || !FChar::IsAlpha(Value[Index - 1]))
            {
                return false;
            }

            const bool bIsLastCharacter = Index == Value.Len() - 1;

            if (bIsLastCharacter)
            {
                continue;
            }

            // Internal apostrophes must be followed by a letter.
            if (!FChar::IsAlpha(Value[Index + 1]))
            {
                return false;
            }

            continue;
        }

        return false;
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

    if (!ContainsOnlyLettersSpacesAndInternalApostrophes(OutNormalizedWord))
    {
        OutErrorMessage = FText::FromString(TEXT("Only letters, spaces, and internal apostrophes are allowed."));

        return EEVInputValidationResult::InvalidCharacters;
    }

    return EEVInputValidationResult::Valid;
}
