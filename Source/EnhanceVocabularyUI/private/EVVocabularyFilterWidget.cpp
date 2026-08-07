#include "EVVocabularyFilterWidget.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "EVVocabularyActiveFilterCategory.h"
#include "EVVocabularyFilterCategory.h"

void UEVVocabularyFilterWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_Background)
    {
        Button_Background->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleBackgroundPressed);
    }
    if (Button_Close)
    {
        Button_Close->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleClosePressed);
    }
    if (Button_FilterSelector)
    {
        Button_FilterSelector->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleFilterSelectorPressed);
    }
    if (Button_ApplyChanges)
    {
        Button_ApplyChanges->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleApplyChangesPressed);
    }
}

void UEVVocabularyFilterWidget::NativeConstruct()
{
    Super::NativeConstruct();
    bCategoriesExpanded = false;
    if (Border_CategoriesPanel)
    {
        Border_CategoriesPanel->SetVisibility(ESlateVisibility::Collapsed);
    }
    PopulateCategories();
    RebuildActiveFilters();
}

void UEVVocabularyFilterWidget::SetInitialCriteria(const FEVVocabularyQueryCriteria& Criteria)
{
    WorkingCriteria = Criteria;
    WorkingCriteria.Normalize();
    PopulateCategories();
    RebuildActiveFilters();
}

void UEVVocabularyFilterWidget::PopulateCategories()
{
    if (!VerticalBox_Categories || !FilterCategoryWidgetClass)
    {
        return;
    }

    VerticalBox_Categories->ClearChildren();
    CategoryWidgets.Reset();

    const TArray<EEVVocabularyFilterCategory> Categories = {EEVVocabularyFilterCategory::PartOfSpeech};

    for (const EEVVocabularyFilterCategory Category : Categories)
    {
        UEVVocabularyFilterCategory* CategoryWidget =
            CreateWidget<UEVVocabularyFilterCategory>(this, FilterCategoryWidgetClass);
        if (!CategoryWidget)
        {
            continue;
        }

        const FEVVocabularyFilterCategorySelection* ExistingSelection = WorkingCriteria.FindCategory(Category);
        const TArray<EEVVocabularyFilterOption> SelectedOptions =
            ExistingSelection ? ExistingSelection->SelectedOptions : TArray<EEVVocabularyFilterOption>{};

        CategoryWidget->ConfigureCategory(Category, EVVocabularyFilter::GetOptionsForCategory(Category),
                                          SelectedOptions);
        CategoryWidget->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::HandleCategorySelectionChanged);
        UVerticalBoxSlot* VBoxSlot = VerticalBox_Categories->AddChildToVerticalBox(CategoryWidget);
        if (VBoxSlot)
        {
            VBoxSlot->SetHorizontalAlignment(HAlign_Fill);
            VBoxSlot->SetVerticalAlignment(VAlign_Top);
            VBoxSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
        }
        CategoryWidgets.Add(Category, CategoryWidget);
    }
}

void UEVVocabularyFilterWidget::RebuildActiveFilters()
{
    UVerticalBox* ActiveFiltersContainer = GetActiveFiltersContainer();
    if (!ActiveFiltersContainer)
    {
        UpdateEmptyState();
        return;
    }

    ActiveFiltersContainer->ClearChildren();

    if (ActiveFilterCategoryWidgetClass)
    {
        for (const FEVVocabularyFilterCategorySelection& Selection : WorkingCriteria.Categories)
        {
            if (!Selection.IsValid())
            {
                continue;
            }

            UEVVocabularyActiveFilterCategory* ActiveWidget =
                CreateWidget<UEVVocabularyActiveFilterCategory>(this, ActiveFilterCategoryWidgetClass);
            if (!ActiveWidget)
            {
                continue;
            }

            ActiveWidget->SetSelection(Selection);
            ActiveWidget->OnCategoryRemoveRequested.AddUniqueDynamic(this,
                                                                     &ThisClass::HandleActiveCategoryRemoveRequested);
            ActiveWidget->OnOptionRemoveRequested.AddUniqueDynamic(this, &ThisClass::HandleActiveOptionRemoveRequested);

            UVerticalBoxSlot* VBoxSlot = ActiveFiltersContainer->AddChildToVerticalBox(ActiveWidget);
            if (VBoxSlot)
            {
                VBoxSlot->SetHorizontalAlignment(HAlign_Fill);
                VBoxSlot->SetVerticalAlignment(VAlign_Top);
                VBoxSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 10.f));
            }
        }
    }

    UpdateEmptyState();
}

void UEVVocabularyFilterWidget::SynchronizeCategoryWidgets()
{
    for (const TPair<EEVVocabularyFilterCategory, UEVVocabularyFilterCategory*>& Pair : CategoryWidgets)
    {
        if (Pair.Value)
        {
            Pair.Value->ApplySelection(WorkingCriteria.FindCategory(Pair.Key));
        }
    }
}

void UEVVocabularyFilterWidget::UpdateEmptyState()
{
    const bool bHasFilters = WorkingCriteria.HasActiveFilters();
    if (TextBlock_NoActiveFilters)
    {
        TextBlock_NoActiveFilters->SetVisibility(bHasFilters ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    }
    if (UVerticalBox* ActiveFiltersContainer = GetActiveFiltersContainer())
    {
        ActiveFiltersContainer->SetVisibility(bHasFilters ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

UVerticalBox* UEVVocabularyFilterWidget::GetActiveFiltersContainer() const
{
    return VerticalBox_ActiveFilters ? VerticalBox_ActiveFilters.Get() : VerticalBox_SelectedFilters.Get();
}

void UEVVocabularyFilterWidget::UpsertSelection(const FEVVocabularyFilterCategorySelection& Selection)
{
    WorkingCriteria.Categories.RemoveAll([&Selection](const FEVVocabularyFilterCategorySelection& Existing)
                                         { return Existing.Category == Selection.Category; });
    if (Selection.IsValid())
    {
        WorkingCriteria.Categories.Add(Selection);
    }
    WorkingCriteria.Normalize();
}

void UEVVocabularyFilterWidget::RemoveCategory(const EEVVocabularyFilterCategory Category)
{
    WorkingCriteria.Categories.RemoveAll([Category](const FEVVocabularyFilterCategorySelection& Selection)
                                         { return Selection.Category == Category; });
    WorkingCriteria.Normalize();
    SynchronizeCategoryWidgets();
    RebuildActiveFilters();
}

void UEVVocabularyFilterWidget::RemoveOption(const EEVVocabularyFilterCategory Category,
                                             const EEVVocabularyFilterOption Option)
{
    FEVVocabularyFilterCategorySelection* Selection = WorkingCriteria.Categories.FindByPredicate(
        [Category](const FEVVocabularyFilterCategorySelection& Existing) { return Existing.Category == Category; });
    if (Selection)
    {
        Selection->SelectedOptions.Remove(Option);
    }
    WorkingCriteria.Normalize();
    SynchronizeCategoryWidgets();
    RebuildActiveFilters();
}

void UEVVocabularyFilterWidget::HandleBackgroundPressed()
{
    bCategoriesExpanded = false;
    if (Border_CategoriesPanel)
    {
        Border_CategoriesPanel->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UEVVocabularyFilterWidget::HandleClosePressed()
{
    OnCloseRequested.Broadcast();
}

void UEVVocabularyFilterWidget::HandleFilterSelectorPressed()
{
    bCategoriesExpanded = !bCategoriesExpanded;
    if (Border_CategoriesPanel)
    {
        Border_CategoriesPanel->SetVisibility(bCategoriesExpanded ? ESlateVisibility::Visible
                                                                  : ESlateVisibility::Collapsed);
    }
}

void UEVVocabularyFilterWidget::HandleApplyChangesPressed()
{
    WorkingCriteria.Normalize();
    OnFiltersApplied.Broadcast(WorkingCriteria);
}

void UEVVocabularyFilterWidget::HandleCategorySelectionChanged(const FEVVocabularyFilterCategorySelection& Selection)
{
    UpsertSelection(Selection);
    RebuildActiveFilters();
}

void UEVVocabularyFilterWidget::HandleActiveCategoryRemoveRequested(const EEVVocabularyFilterCategory Category)
{
    RemoveCategory(Category);
}

void UEVVocabularyFilterWidget::HandleActiveOptionRemoveRequested(const EEVVocabularyFilterCategory Category,
                                                                  const EEVVocabularyFilterOption Option)
{
    RemoveOption(Category, Option);
}
