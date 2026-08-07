#include "EVVocabularyTranslationOption.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "EVCheckBoxWidget.h"

void UEVVocabularyTranslationOption::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_Option)
    {
        Button_Option->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleOptionButtonPressed);
    }
    if (WBP_CheckBox_Option)
    {
        WBP_CheckBox_Option->OnCheckStateChanged.AddUniqueDynamic(this, &ThisClass::HandleCheckStateChanged);
    }
}

void UEVVocabularyTranslationOption::ConfigureOption(const EEVVocabularyTranslationLanguage InLanguage,
                                                     const bool bInSelected)
{
    Language = InLanguage;
    if (TextBlock_OptionName)
    {
        TextBlock_OptionName->SetText(EVVocabularyLanguage::GetTranslationLanguageDisplayText(Language));
        TextBlock_OptionName->SetAutoWrapText(true);
    }
    SetSelected(bInSelected, false);
}

void UEVVocabularyTranslationOption::SetSelected(const bool bInSelected, const bool bBroadcastChange)
{
    const bool bChanged = bSelected != bInSelected;
    bSelected = bInSelected;

    bApplyingState = true;
    if (WBP_CheckBox_Option)
    {
        WBP_CheckBox_Option->SetChecked(bSelected, false);
    }
    bApplyingState = false;

    if (bChanged && bBroadcastChange)
    {
        OnSelectionChanged.Broadcast(Language, bSelected);
    }
}

void UEVVocabularyTranslationOption::HandleOptionButtonPressed()
{
    SetSelected(!bSelected, true);
}

void UEVVocabularyTranslationOption::HandleCheckStateChanged(const bool bIsChecked)
{
    if (!bApplyingState)
    {
        SetSelected(bIsChecked, true);
    }
}
