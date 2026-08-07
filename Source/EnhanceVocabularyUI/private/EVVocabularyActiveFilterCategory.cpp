#include "EVVocabularyActiveFilterCategory.h"

#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "EVVocabularyFilterChipWidget.h"

void UEVVocabularyActiveFilterCategory::SetSelection(const FEVVocabularyFilterCategorySelection& InSelection)
{
    Selection = InSelection;
    RebuildChips();
}

void UEVVocabularyActiveFilterCategory::RebuildChips()
{
    CategoryChip = nullptr;
    OptionByChip.Reset();

    if (WrapBox_Category)
    {
        WrapBox_Category->ClearChildren();
    }

    if (WrapBox_Options)
    {
        WrapBox_Options->ClearChildren();
    }

    if (!Selection.IsValid())
    {
        return;
    }

    if (!FilterChipWidgetClass)
    {
        UE_LOG(LogTemp, Error,
               TEXT("Vocabulary Active Filter Category: "
                    "FilterChipWidgetClass is not assigned."));
        return;
    }

    //
    // CATEGORY CHIP
    //

    CategoryChip = CreateChip(EVVocabularyFilter::GetCategoryDisplayText(Selection.Category));

    if (CategoryChip && WrapBox_Category)
    {
        UWrapBoxSlot* CategoryWrapBoxSlot = WrapBox_Category->AddChildToWrapBox(CategoryChip);

        if (CategoryWrapBoxSlot)
        {
            CategoryWrapBoxSlot->SetPadding(FMargin(0.f, 0.f, 6.f, 4.f));
        }
    }

    //
    // OPTION CHIPS
    //

    if (!WrapBox_Options)
    {
        return;
    }

    for (const EEVVocabularyFilterOption Option : Selection.SelectedOptions)
    {
        UEVVocabularyFilterChipWidget* OptionChip = CreateChip(EVVocabularyFilter::GetOptionDisplayText(Option));

        if (!OptionChip)
        {
            continue;
        }

        OptionByChip.Add(OptionChip, Option);

        UWrapBoxSlot* OptionWrapBoxSlot = WrapBox_Options->AddChildToWrapBox(OptionChip);

        if (OptionWrapBoxSlot)
        {
            OptionWrapBoxSlot->SetPadding(FMargin(0.f, 0.f, 6.f, 4.f));
        }
    }
}

UEVVocabularyFilterChipWidget* UEVVocabularyActiveFilterCategory::CreateChip(const FText& Label)
{
    if (!FilterChipWidgetClass)
    {
        return nullptr;
    }

    UEVVocabularyFilterChipWidget* Chip =
        CreateWidget<UEVVocabularyFilterChipWidget>(GetOwningPlayer(), FilterChipWidgetClass);

    if (!Chip)
    {
        return nullptr;
    }

    Chip->SetChipText(Label);

    Chip->OnRemoveRequested.AddUniqueDynamic(this, &ThisClass::HandleChipRemoveRequested);

    return Chip;
}

void UEVVocabularyActiveFilterCategory::HandleChipRemoveRequested(UEVVocabularyFilterChipWidget* ChipWidget)
{
    if (!ChipWidget)
    {
        return;
    }

    //
    // CATEGORY REMOVAL
    //
    // Removing the category means removing every option belonging
    // to this category as well.
    //

    if (ChipWidget == CategoryChip)
    {
        OnCategoryRemoveRequested.Broadcast(Selection.Category);

        return;
    }

    //
    // SINGLE OPTION REMOVAL
    //

    if (const EEVVocabularyFilterOption* Option = OptionByChip.Find(ChipWidget))
    {
        OnOptionRemoveRequested.Broadcast(Selection.Category, *Option);
    }
}