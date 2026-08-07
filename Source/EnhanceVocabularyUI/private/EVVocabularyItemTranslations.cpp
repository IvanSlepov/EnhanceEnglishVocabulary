// Fill out your copyright notice in the Description page of Project Settings.

#include "EVVocabularyItemTranslations.h"

#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "EVVocabularyLanguageTypes.h"
#include "EVVocabularyLanguageUiData.h"

void UEVVocabularyItemTranslations::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleTranslationValueChanged);

    OnValueCommitted.AddUniqueDynamic(this, &ThisClass::HandleTranslationValueCommitted);
}

void UEVVocabularyItemTranslations::SetTranslation(const FEVVocabularyTranslation& InTranslation)
{
    Translation = InTranslation;

    SetValue(Translation.TranslationText);
    ApplyLanguageFlag();
}

const FEVVocabularyTranslation& UEVVocabularyItemTranslations::GetTranslation() const
{
    return Translation;
}

void UEVVocabularyItemTranslations::HandleTranslationValueChanged(UEVVocabularyValueItemWidgetBase* ItemWidget,
                                                                  const FString& NewValue)
{
    if (ItemWidget != this)
    {
        return;
    }

    Translation.TranslationText = NewValue;
}

void UEVVocabularyItemTranslations::HandleTranslationValueCommitted(UEVVocabularyValueItemWidgetBase* ItemWidget,
                                                                    const FString& CommittedValue,
                                                                    ETextCommit::Type CommitMethod)
{
    if (ItemWidget != this)
    {
        return;
    }

    Translation.TranslationText = CommittedValue;
}

void UEVVocabularyItemTranslations::ApplyLanguageFlag()
{
    if (!Image_LanguageCountryFlag)
    {
        return;
    }

    EEVVocabularyTranslationLanguage Language = EEVVocabularyTranslationLanguage::None;
    if (!EVVocabularyLanguage::TryParseTranslationLanguage(Translation.TargetLanguage, Language))
    {
        Image_LanguageCountryFlag->SetVisibility(ESlateVisibility::Hidden);
        return;
    }

    UTexture2D* Flag = LanguageUiData ? LanguageUiData->GetFlagTexture(Language) : nullptr;
    if (!Flag)
    {
        Image_LanguageCountryFlag->SetVisibility(ESlateVisibility::Hidden);
        return;
    }

    Image_LanguageCountryFlag->SetBrushFromTexture(Flag, false);
    Image_LanguageCountryFlag->SetVisibility(ESlateVisibility::Visible);
}
