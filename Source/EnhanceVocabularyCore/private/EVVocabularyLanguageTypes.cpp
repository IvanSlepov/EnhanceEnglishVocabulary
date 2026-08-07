#include "EVVocabularyLanguageTypes.h"

namespace
{
FString NormalizeLanguageToken(const FString& Value)
{
    FString Result = Value.TrimStartAndEnd().ToLower();
    Result.ReplaceInline(TEXT("_"), TEXT("-"));
    return Result;
}
} // namespace

void FEVVocabularyLanguagePreferences::Normalize()
{
    if (DatabaseContext == EEVVocabularyDBContext::None)
    {
        DatabaseContext = EEVVocabularyDBContext::EnglishUSA;
    }

    TArray<EEVVocabularyTranslationLanguage> Normalized;
    for (const EEVVocabularyTranslationLanguage Language : SelectedTranslations)
    {
        if (Language == EEVVocabularyTranslationLanguage::None ||
            EVVocabularyLanguage::IsTranslationEquivalentToDatabaseContext(Language, DatabaseContext) ||
            Normalized.Contains(Language))
        {
            continue;
        }
        Normalized.Add(Language);
    }
    SelectedTranslations = MoveTemp(Normalized);
}

namespace EVVocabularyLanguage
{
const TArray<EEVVocabularyDBContext>& GetAvailableDatabaseContexts()
{
    static const TArray<EEVVocabularyDBContext> Contexts = {EEVVocabularyDBContext::EnglishUSA};
    return Contexts;
}

const TArray<EEVVocabularyTranslationLanguage>& GetAvailableTranslationLanguages()
{
    static const TArray<EEVVocabularyTranslationLanguage> Languages = {
        EEVVocabularyTranslationLanguage::EnglishUSA, EEVVocabularyTranslationLanguage::EnglishUK,
        EEVVocabularyTranslationLanguage::Russian,    EEVVocabularyTranslationLanguage::Ukrainian,
        EEVVocabularyTranslationLanguage::Spanish,    EEVVocabularyTranslationLanguage::German,
        EEVVocabularyTranslationLanguage::French,     EEVVocabularyTranslationLanguage::Italian};
    return Languages;
}

FText GetDatabaseContextDisplayText(const EEVVocabularyDBContext Context)
{
    switch (Context)
    {
    case EEVVocabularyDBContext::EnglishUSA:
        return NSLOCTEXT("EVVocabularyLanguage", "ContextEnglishUSA", "English USA");
    default:
        return NSLOCTEXT("EVVocabularyLanguage", "ContextNone", "None");
    }
}

FText GetTranslationLanguageDisplayText(const EEVVocabularyTranslationLanguage Language)
{
    switch (Language)
    {
    case EEVVocabularyTranslationLanguage::EnglishUSA:
        return NSLOCTEXT("EVVocabularyLanguage", "TranslationEnglishUSA", "English USA");
    case EEVVocabularyTranslationLanguage::EnglishUK:
        return NSLOCTEXT("EVVocabularyLanguage", "TranslationEnglishUK", "English UK");
    case EEVVocabularyTranslationLanguage::Russian:
        return NSLOCTEXT("EVVocabularyLanguage", "TranslationRussian", "Russian");
    case EEVVocabularyTranslationLanguage::Ukrainian:
        return NSLOCTEXT("EVVocabularyLanguage", "TranslationUkrainian", "Ukrainian");
    case EEVVocabularyTranslationLanguage::Spanish:
        return NSLOCTEXT("EVVocabularyLanguage", "TranslationSpanish", "Spanish");
    case EEVVocabularyTranslationLanguage::German:
        return NSLOCTEXT("EVVocabularyLanguage", "TranslationGerman", "German");
    case EEVVocabularyTranslationLanguage::French:
        return NSLOCTEXT("EVVocabularyLanguage", "TranslationFrench", "French");
    case EEVVocabularyTranslationLanguage::Italian:
        return NSLOCTEXT("EVVocabularyLanguage", "TranslationItalian", "Italian");
    default:
        return NSLOCTEXT("EVVocabularyLanguage", "TranslationNone", "None");
    }
}

FString GetDatabaseContextId(const EEVVocabularyDBContext Context)
{
    return Context == EEVVocabularyDBContext::EnglishUSA ? TEXT("en-US") : FString();
}

FString GetTranslationStorageCode(const EEVVocabularyTranslationLanguage Language)
{
    switch (Language)
    {
    case EEVVocabularyTranslationLanguage::EnglishUSA:
        return TEXT("en-US");
    case EEVVocabularyTranslationLanguage::EnglishUK:
        return TEXT("en-GB");
    case EEVVocabularyTranslationLanguage::Russian:
        return TEXT("ru");
    case EEVVocabularyTranslationLanguage::Ukrainian:
        return TEXT("uk");
    case EEVVocabularyTranslationLanguage::Spanish:
        return TEXT("es");
    case EEVVocabularyTranslationLanguage::German:
        return TEXT("de");
    case EEVVocabularyTranslationLanguage::French:
        return TEXT("fr");
    case EEVVocabularyTranslationLanguage::Italian:
        return TEXT("it");
    default:
        return FString();
    }
}

FString GetDatabaseContextWebLanguageCode(const EEVVocabularyDBContext Context)
{
    // MyMemory and the current dictionary provider use broad source-language codes.
    return Context == EEVVocabularyDBContext::EnglishUSA ? TEXT("en") : FString();
}

FString GetTranslationWebLanguageCode(const EEVVocabularyTranslationLanguage Language)
{
    switch (Language)
    {
    case EEVVocabularyTranslationLanguage::EnglishUSA:
        return TEXT("en-US");
    case EEVVocabularyTranslationLanguage::EnglishUK:
        return TEXT("en-GB");
    case EEVVocabularyTranslationLanguage::Russian:
        return TEXT("ru");
    case EEVVocabularyTranslationLanguage::Ukrainian:
        return TEXT("uk");
    case EEVVocabularyTranslationLanguage::Spanish:
        return TEXT("es");
    case EEVVocabularyTranslationLanguage::German:
        return TEXT("de");
    case EEVVocabularyTranslationLanguage::French:
        return TEXT("fr");
    case EEVVocabularyTranslationLanguage::Italian:
        return TEXT("it");
    default:
        return FString();
    }
}

FString GetDatabaseContextPronunciationLanguageCode(const EEVVocabularyDBContext Context)
{
    return Context == EEVVocabularyDBContext::EnglishUSA ? TEXT("en") : FString();
}

bool TryParseDatabaseContext(const FString& Value, EEVVocabularyDBContext& OutContext)
{
    const FString Token = NormalizeLanguageToken(Value);
    if (Token == TEXT("en-us") || Token == TEXT("english usa") || Token == TEXT("english us"))
    {
        OutContext = EEVVocabularyDBContext::EnglishUSA;
        return true;
    }
    OutContext = EEVVocabularyDBContext::None;
    return false;
}

bool TryParseTranslationLanguage(const FString& Value, EEVVocabularyTranslationLanguage& OutLanguage)
{
    const FString Token = NormalizeLanguageToken(Value);
    if (Token == TEXT("en-us") || Token == TEXT("english usa") || Token == TEXT("english us") || Token == TEXT("en"))
        OutLanguage = EEVVocabularyTranslationLanguage::EnglishUSA;
    else if (Token == TEXT("en-gb") || Token == TEXT("english uk") || Token == TEXT("english gb"))
        OutLanguage = EEVVocabularyTranslationLanguage::EnglishUK;
    else if (Token == TEXT("ru") || Token == TEXT("russian"))
        OutLanguage = EEVVocabularyTranslationLanguage::Russian;
    else if (Token == TEXT("uk") || Token == TEXT("ua") || Token == TEXT("ukrainian"))
        OutLanguage = EEVVocabularyTranslationLanguage::Ukrainian;
    else if (Token == TEXT("es") || Token == TEXT("spanish"))
        OutLanguage = EEVVocabularyTranslationLanguage::Spanish;
    else if (Token == TEXT("de") || Token == TEXT("german"))
        OutLanguage = EEVVocabularyTranslationLanguage::German;
    else if (Token == TEXT("fr") || Token == TEXT("french"))
        OutLanguage = EEVVocabularyTranslationLanguage::French;
    else if (Token == TEXT("it") || Token == TEXT("italian"))
        OutLanguage = EEVVocabularyTranslationLanguage::Italian;
    else
    {
        OutLanguage = EEVVocabularyTranslationLanguage::None;
        return false;
    }
    return true;
}

bool IsTranslationEquivalentToDatabaseContext(const EEVVocabularyTranslationLanguage Language,
                                              const EEVVocabularyDBContext Context)
{
    return Context == EEVVocabularyDBContext::EnglishUSA && Language == EEVVocabularyTranslationLanguage::EnglishUSA;
}

bool TryResolveDatabaseContextForTranslation(const EEVVocabularyTranslationLanguage Language,
                                             EEVVocabularyDBContext& OutContext)
{
    if (Language == EEVVocabularyTranslationLanguage::EnglishUSA)
    {
        OutContext = EEVVocabularyDBContext::EnglishUSA;
        return true;
    }
    OutContext = EEVVocabularyDBContext::None;
    return false;
}
} // namespace EVVocabularyLanguage
