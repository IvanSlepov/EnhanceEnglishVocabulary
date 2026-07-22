#pragma once

#include "CoreMinimal.h"

namespace EVVocabularyUiStyle
{
inline const FSlateColor& GetNormalWrodEntryTextFontColor()
{
    static const FSlateColor Color(FLinearColor::Black);
    return Color;
}

inline const FSlateColor& GetMissingWrodEntryTextFontColor()
{
    static const FSlateColor Color(FLinearColor::Red);
    return Color;
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