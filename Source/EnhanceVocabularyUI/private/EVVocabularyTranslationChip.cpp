#include "EVVocabularyTranslationChip.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Components/TextBlock.h"
#include "EVVocabularyLanguageUiData.h"

void UEVVocabularyTranslationChip::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    if (Button_Remove)
    {
        Button_Remove->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleRemovePressed);
    }
    ApplyLanguageVisuals();
}

void UEVVocabularyTranslationChip::SetLanguage(const EEVVocabularyTranslationLanguage InLanguage)
{
    Language = InLanguage;
    ApplyLanguageVisuals();
}

void UEVVocabularyTranslationChip::ApplyLanguageVisuals()
{
    if (TextBlock_Value)
    {
        TextBlock_Value->SetText(EVVocabularyLanguage::GetTranslationLanguageDisplayText(Language));
    }

    if (Image_LanguageCountryFlag)
    {
        UTexture2D* Flag = LanguageUiData ? LanguageUiData->GetFlagTexture(Language) : nullptr;
        if (Flag)
        {
            Image_LanguageCountryFlag->SetBrushFromTexture(Flag, false);
            Image_LanguageCountryFlag->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            Image_LanguageCountryFlag->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void UEVVocabularyTranslationChip::HandleRemovePressed()
{
    if (Language != EEVVocabularyTranslationLanguage::None)
    {
        OnRemoveRequested.Broadcast(Language);
    }
}
