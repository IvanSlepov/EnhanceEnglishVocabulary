#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVVocabularyFilterTypes.h"
#include "EVVocabularyFilterWidgetProvider.h"
#include "EVVocabularyFilterWidget.generated.h"

class UButton;
class UBorder;
class UTextBlock;
class UVerticalBox;
class UEVVocabularyFilterCategory;
class UEVVocabularyActiveFilterCategory;

UCLASS()
class ENHANCEVOCABULARYUI_API UEVVocabularyFilterWidget : public UUserWidget, public IEVVocabularyFilterWidgetProvider
{
    GENERATED_BODY()

public:
    virtual void SetInitialCriteria(const FEVVocabularyQueryCriteria& Criteria) override;
    virtual FOnVocabularyFiltersApplied& GetFiltersAppliedEvent() override
    {
        return OnFiltersApplied;
    }
    virtual FOnVocabularyFilterWidgetCloseRequested& GetCloseRequestedEvent() override
    {
        return OnCloseRequested;
    }

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

private:
    void PopulateCategories();
    void RebuildActiveFilters();
    void SynchronizeCategoryWidgets();
    void UpdateEmptyState();
    UVerticalBox* GetActiveFiltersContainer() const;
    void UpsertSelection(const FEVVocabularyFilterCategorySelection& Selection);
    void RemoveCategory(EEVVocabularyFilterCategory Category);
    void RemoveOption(EEVVocabularyFilterCategory Category, EEVVocabularyFilterOption Option);

    UFUNCTION()
    void HandleBackgroundPressed();

    UFUNCTION()
    void HandleClosePressed();

    UFUNCTION()
    void HandleFilterSelectorPressed();

    UFUNCTION()
    void HandleApplyChangesPressed();

    UFUNCTION()
    void HandleCategorySelectionChanged(const FEVVocabularyFilterCategorySelection& Selection);

    UFUNCTION()
    void HandleActiveCategoryRemoveRequested(EEVVocabularyFilterCategory Category);

    UFUNCTION()
    void HandleActiveOptionRemoveRequested(EEVVocabularyFilterCategory Category, EEVVocabularyFilterOption Option);

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Button_Background = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Close = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_FilterSelector = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> Border_CategoriesPanel = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> VerticalBox_Categories = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> Border_SelectedFiltersPanel = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TextBlock_NoActiveFilters = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UVerticalBox> VerticalBox_ActiveFilters = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UVerticalBox> VerticalBox_SelectedFilters = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_ApplyChanges = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vocabulary Filters|Widget Classes",
              meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UEVVocabularyFilterCategory> FilterCategoryWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vocabulary Filters|Widget Classes",
              meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UEVVocabularyActiveFilterCategory> ActiveFilterCategoryWidgetClass;

    UPROPERTY(Transient)
    FEVVocabularyQueryCriteria WorkingCriteria;

    UPROPERTY(Transient)
    bool bCategoriesExpanded = false;

    TMap<EEVVocabularyFilterCategory, UEVVocabularyFilterCategory*> CategoryWidgets;

    UPROPERTY(BlueprintAssignable, Category = "Vocabulary Filters|Events")
    FOnVocabularyFiltersApplied OnFiltersApplied;

    UPROPERTY(BlueprintAssignable, Category = "Vocabulary Filters|Events")
    FOnVocabularyFilterWidgetCloseRequested OnCloseRequested;
};
