#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVVocabularyFilterTypes.h"
#include "EVVocabularyActiveFilterCategory.generated.h"

class UWrapBox;
class UEVVocabularyFilterChipWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveFilterCategoryRemoveRequested, EEVVocabularyFilterCategory,
                                            Category);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActiveFilterOptionRemoveRequested, EEVVocabularyFilterCategory,
                                             Category, EEVVocabularyFilterOption, Option);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVVocabularyActiveFilterCategory : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     * Assigns one active category and its selected options.
     *
     * Rebuilds:
     * - the removable category chip;
     * - all removable option chips.
     */
    void SetSelection(const FEVVocabularyFilterCategorySelection& InSelection);

    UPROPERTY(BlueprintAssignable, Category = "Vocabulary Filters|Events")
    FOnActiveFilterCategoryRemoveRequested OnCategoryRemoveRequested;

    UPROPERTY(BlueprintAssignable, Category = "Vocabulary Filters|Events")
    FOnActiveFilterOptionRemoveRequested OnOptionRemoveRequested;

private:
    void RebuildChips();

    UEVVocabularyFilterChipWidget* CreateChip(const FText& Label);

    UFUNCTION()
    void HandleChipRemoveRequested(UEVVocabularyFilterChipWidget* ChipWidget);

private:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWrapBox> WrapBox_Category = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWrapBox> WrapBox_Options = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vocabulary Filters|Widget Classes",
              meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UEVVocabularyFilterChipWidget> FilterChipWidgetClass;

    UPROPERTY(Transient)
    FEVVocabularyFilterCategorySelection Selection;

    /**
     * The single chip representing the category itself.
     *
     * Example:
     * [ Part of speech X ]
     */
    UPROPERTY(Transient)
    TObjectPtr<UEVVocabularyFilterChipWidget> CategoryChip = nullptr;

    /**
     * Maps every generated option chip back to the enum it represents.
     */
    TMap<UEVVocabularyFilterChipWidget*, EEVVocabularyFilterOption> OptionByChip;
};