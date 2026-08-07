#include "EVVocabularyFilterOption.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "EVCheckBoxWidget.h"

void UEVVocabularyFilterOption::NativeOnInitialized()
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

void UEVVocabularyFilterOption::ConfigureOption(const EEVVocabularyFilterOption InOption, const bool bInSelected)
{
    Option = InOption;
    if (TextBlock_OptionName)
    {
        TextBlock_OptionName->SetText(EVVocabularyFilter::GetOptionDisplayText(Option));
        TextBlock_OptionName->SetAutoWrapText(true);
    }
    SetSelected(bInSelected, false);
}

void UEVVocabularyFilterOption::SetSelected(const bool bInSelected, const bool bBroadcastChange)
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
        OnSelectionChanged.Broadcast(Option, bSelected);
    }
}

void UEVVocabularyFilterOption::HandleOptionButtonPressed()
{
    SetSelected(!bSelected, true);
}

void UEVVocabularyFilterOption::HandleCheckStateChanged(const bool bIsChecked)
{
    if (!bApplyingState)
    {
        SetSelected(bIsChecked, true);
    }
}
