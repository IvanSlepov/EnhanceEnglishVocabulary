#include "EVVocabularyLanguageUiData.h"

#include "Engine/Texture2D.h"

UTexture2D* UEVVocabularyLanguageUiData::GetFlagTexture(const EEVVocabularyTranslationLanguage Language) const
{
    if (const TObjectPtr<UTexture2D>* Found = LanguageFlags.Find(Language))
    {
        return Found->Get();
    }
    return nullptr;
}
