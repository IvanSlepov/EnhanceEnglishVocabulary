#pragma once

#include "CoreMinimal.h"
#include "EVVocabularyTypes.h"

/**
 * Shared helpers for keeping vocabulary translations atomic.
 *
 * One FEVVocabularyTranslation represents one independently actionable
 * translation value. Provider/import/legacy packed values are expanded into
 * separate records while retaining their language/provider metadata.
 */
namespace EVVocabularyTranslationUtils
{
/** Splits provider/import packed alternatives on comma, semicolon or line breaks. */
ENHANCEVOCABULARYCORE_API void SplitTranslationValues(const FString& Source, TArray<FString>& OutValues);

/** Expands one possibly packed translation into atomic translation records. */
ENHANCEVOCABULARYCORE_API void ExpandTranslation(const FEVVocabularyTranslation& Source,
                                                 TArray<FEVVocabularyTranslation>& OutTranslations);

/** Expands, de-duplicates and normalizes DisplayOrder for a translation collection. */
ENHANCEVOCABULARYCORE_API void NormalizeTranslations(TArray<FEVVocabularyTranslation>& Translations);
} // namespace EVVocabularyTranslationUtils
