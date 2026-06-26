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
    if (Input.IsEmpty())
    {
        return false;
    }

    for (int32 Index = 0; Index < Input.Len(); ++Index)
    {
        const TCHAR Char = Input[Index];

        const bool bIsLetter = FChar::IsAlpha(Char);
        const bool bIsSpace = FChar::IsWhitespace(Char);
        const bool bIsHyphen = Char == TEXT('-');

        if (!bIsLetter && !bIsSpace && !bIsHyphen)
        {
            return false;
        }

        const bool bIsSeparator = bIsSpace || bIsHyphen;

        if (bIsSeparator)
        {
            const bool bIsAtEdge = Index == 0 || Index == Input.Len() - 1;
            if (bIsAtEdge)
            {
                return false;
            }

            const TCHAR PreviousChar = Input[Index - 1];
            const TCHAR NextChar = Input[Index + 1];

            const bool bPreviousIsSeparator = FChar::IsWhitespace(PreviousChar) || PreviousChar == TEXT('-');

            const bool bNextIsSeparator = FChar::IsWhitespace(NextChar) || NextChar == TEXT('-');

            if (bPreviousIsSeparator || bNextIsSeparator)
            {
                return false;
            }
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
