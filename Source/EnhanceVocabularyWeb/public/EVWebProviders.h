#pragma once

#include "CoreMinimal.h"

namespace EVWebProviders
{
// Definition + Usage
constexpr TCHAR FreeDictionary[] = TEXT("https://api.dictionaryapi.dev/api/v2/entries/en/");

// Translation
constexpr TCHAR MyMemory[] = TEXT("https://api.mymemory.translated.net/get");

// Metadata
constexpr TCHAR Datamuse[] = TEXT("https://api.datamuse.com/words");

// PAID SERVICE !!! Let's try it later.
// The missing part is 'en-gb?q=walk', where en-gb - language of the word we seek; ?q=someword - the way we pass the
// word we seek
constexpr TCHAR OxfordDictionaries[] = TEXT("https://od-api.oxforddictionaries.com/api/v2/words/");

} // namespace EVWebProviders