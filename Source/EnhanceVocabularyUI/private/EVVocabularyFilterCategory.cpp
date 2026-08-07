#include "EVVocabularyFilterCategory.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "EVCheckBoxWidget.h"
#include "EVVocabularyFilterOption.h"

void UEVVocabularyFilterCategory::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_Category)
    {
        Button_Category->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleCategoryButtonPressed);
    }
    if (WBP_CheckBox_Category)
    {
        WBP_CheckBox_Category->OnCheckStateChanged.AddUniqueDynamic(this, &ThisClass::HandleCategoryCheckStateChanged);
    }

    SetExpanded(false);
}

void UEVVocabularyFilterCategory::ConfigureCategory(const EEVVocabularyFilterCategory InCategory,
                                                    const TArray<EEVVocabularyFilterOption>& InOptions,
                                                    const TArray<EEVVocabularyFilterOption>& InSelectedOptions)
{
    Category = InCategory;
    AvailableOptions = InOptions;
    CurrentSelection.Category = Category;
    CurrentSelection.SelectedOptions = InSelectedOptions;
    CurrentSelection.SelectedOptions.RemoveAll([this](const EEVVocabularyFilterOption Option)
                                               { return !AvailableOptions.Contains(Option); });

    if (TextBlock_CategoryName)
    {
        TextBlock_CategoryName->SetText(EVVocabularyFilter::GetCategoryDisplayText(Category));
        TextBlock_CategoryName->SetAutoWrapText(true);
    }

    PopulateOptions();
    RefreshCategoryCheckState();
}

void UEVVocabularyFilterCategory::ApplySelection(const FEVVocabularyFilterCategorySelection* Selection)
{
    CurrentSelection.Category = Category;
    CurrentSelection.SelectedOptions = Selection ? Selection->SelectedOptions : TArray<EEVVocabularyFilterOption>{};
    CurrentSelection.SelectedOptions.RemoveAll([this](const EEVVocabularyFilterOption Option)
                                               { return !AvailableOptions.Contains(Option); });
    PopulateOptions();
    RefreshCategoryCheckState();
}

void UEVVocabularyFilterCategory::PopulateOptions()
{
    if (!VerticalBox_Options || !FilterOptionWidgetClass)
    {
        return;
    }

    VerticalBox_Options->ClearChildren();

    for (const EEVVocabularyFilterOption Option : AvailableOptions)
    {
        UEVVocabularyFilterOption* OptionWidget =
            CreateWidget<UEVVocabularyFilterOption>(this, FilterOptionWidgetClass);
        if (!OptionWidget)
        {
            continue;
        }

        OptionWidget->ConfigureOption(Option, CurrentSelection.SelectedOptions.Contains(Option));
        OptionWidget->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::HandleOptionSelectionChanged);
        UVerticalBoxSlot* VBoxSlot = VerticalBox_Options->AddChildToVerticalBox(OptionWidget);
        if (VBoxSlot)
        {
            VBoxSlot->SetHorizontalAlignment(HAlign_Fill);
            VBoxSlot->SetVerticalAlignment(VAlign_Top);
            VBoxSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));
        }
    }
}

void UEVVocabularyFilterCategory::SetExpanded(const bool bInExpanded)
{
    bExpanded = bInExpanded;
    if (Border_OptionsPanel)
    {
        Border_OptionsPanel->SetVisibility(bExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UEVVocabularyFilterCategory::RefreshCategoryCheckState()
{
    bApplyingState = true;
    if (WBP_CheckBox_Category)
    {
        WBP_CheckBox_Category->SetChecked(CurrentSelection.IsValid(), false);
    }
    bApplyingState = false;
}

void UEVVocabularyFilterCategory::HandleCategoryButtonPressed()
{
    SetExpanded(!bExpanded);
}

void UEVVocabularyFilterCategory::HandleCategoryCheckStateChanged(const bool bIsChecked)
{
    if (bApplyingState)
    {
        return;
    }

    // A category cannot be active without at least one option. The category check control therefore
    // opens/closes the options list rather than selecting an empty category.
    SetExpanded(bIsChecked || !bExpanded);
    RefreshCategoryCheckState();
}

void UEVVocabularyFilterCategory::HandleOptionSelectionChanged(const EEVVocabularyFilterOption Option,
                                                               const bool bSelected)
{
    if (bSelected)
    {
        CurrentSelection.SelectedOptions.AddUnique(Option);
    }
    else
    {
        CurrentSelection.SelectedOptions.Remove(Option);
    }

    RefreshCategoryCheckState();
    OnSelectionChanged.Broadcast(CurrentSelection);
}
