#pragma once

#include "CoreMinimal.h"

namespace EVVocabularyUiStyle
{
inline const FLinearColor& GetNormalWordTextLinearColor()
{
    return FLinearColor::Black;
}

inline const FSlateColor& GetNormalWrodEntryTextFontColor()
{
    static const FSlateColor Color(FLinearColor::Black);
    return Color;
}

inline const FSlateColor& GetWordValueTextFontColor()
{
    static const FSlateColor Color(FLinearColor(FColor(0x6A, 0x1B, 0x9A, 0xFF)));
    return Color;
}

inline const FSlateColor& GetNormalTranscriptionTextFontColor()
{
    static const FSlateColor Color(FLinearColor(FColor(0xFF, 0x6D, 0x00, 0xFF)));
    return Color;
}

inline const FLinearColor& GetMissingWordTextLinearColor()
{
    return FLinearColor::Red;
}

inline const FSlateColor& GetMissingWrodEntryTextFontColor()
{
    static const FSlateColor Color(FLinearColor::Red);
    return Color;
}

/**
 * Subtle background used by pressable translation/relation values in
 * Review Words. It intentionally remains close to white so the values read
 * as part of the meaning while still looking interactive.
 */
inline const FSlateColor& GetReviewValueItemBackgroundTint()
{
    static const FSlateColor Color(FLinearColor(0.818f, 0.818f, 0.818f, 0.227f));
    return Color;
}

inline const FSlateColor& GetInactiveFiltersTextFontColor()
{
    static const FSlateColor Color(FLinearColor(FColor(0x00, 0x97, 0x21, 0xFF)));
    return Color;
}

inline const FSlateColor& GetActiveFiltersTextFontColor()
{
    static const FSlateColor Color(FLinearColor::Red);
    return Color;
}

inline const FSlateColor& GetInactiveTranslationsTextFontColor()
{
    return GetInactiveFiltersTextFontColor();
}

inline const FSlateColor& GetActiveTranslationsTextFontColor()
{
    return GetActiveFiltersTextFontColor();
}

inline FString BuildWrappedWordForDisplay(const FString& Word, int32 BreakInterval = 14)
{
    if (Word.IsEmpty() || BreakInterval <= 0)
    {
        return Word;
    }

    FString DisplayWord;
    DisplayWord.Reserve(Word.Len() + Word.Len() / BreakInterval);

    int32 CharactersSinceBreak = 0;

    for (int32 Index = 0; Index < Word.Len(); ++Index)
    {
        const TCHAR Character = Word[Index];

        DisplayWord.AppendChar(Character);

        if (FChar::IsWhitespace(Character) || Character == TEXT('-') || Character == TEXT('\''))
        {
            CharactersSinceBreak = 0;
            continue;
        }

        ++CharactersSinceBreak;

        const bool bHasMoreCharacters = Index < Word.Len() - 1;

        if (CharactersSinceBreak >= BreakInterval && bHasMoreCharacters)
        {
            DisplayWord.AppendChar(TEXT('\u200B'));
            CharactersSinceBreak = 0;
        }
    }

    return DisplayWord;
}
} // namespace EVVocabularyUiStyle