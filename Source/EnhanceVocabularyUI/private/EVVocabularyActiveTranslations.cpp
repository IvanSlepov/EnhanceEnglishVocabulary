#include "EVVocabularyActiveTranslations.h"

#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "EVVocabularyTranslationChip.h"

void UEVVocabularyActiveTranslations::SetTranslations(const TArray<EEVVocabularyTranslationLanguage>& InTranslations)
{
    Translations = InTranslations;
    RebuildChips();
}

void UEVVocabularyActiveTranslations::RebuildChips()
{
    if (!WrapBox_TranslationOptions)
    {
        return;
    }

    WrapBox_TranslationOptions->ClearChildren();
    if (!TranslationChipWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TranslationChipWidgetClass is not assigned in WBP_VocabularyActiveTranslations."));
        return;
    }

    for (const EEVVocabularyTranslationLanguage Language : Translations)
    {
        UEVVocabularyTranslationChip* Chip =
            CreateWidget<UEVVocabularyTranslationChip>(GetOwningPlayer(), TranslationChipWidgetClass);
        if (!Chip)
        {
            continue;
        }
        Chip->SetLanguage(Language);
        Chip->OnRemoveRequested.AddUniqueDynamic(this, &ThisClass::HandleChipRemoveRequested);

        if (UWrapBoxSlot* ChipSlot = WrapBox_TranslationOptions->AddChildToWrapBox(Chip))
        {
            ChipSlot->SetPadding(FMargin(0.f, 0.f, 6.f, 4.f));
        }
    }
}

void UEVVocabularyActiveTranslations::HandleChipRemoveRequested(const EEVVocabularyTranslationLanguage Language)
{
    OnTranslationRemoveRequested.Broadcast(Language);
}
