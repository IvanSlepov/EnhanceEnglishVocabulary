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
} // namespace EVVocabularyUiStyle