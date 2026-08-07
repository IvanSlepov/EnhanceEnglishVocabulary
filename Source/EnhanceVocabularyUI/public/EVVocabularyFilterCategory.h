#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVVocabularyFilterTypes.h"
#include "EVVocabularyFilterCategory.generated.h"

class UButton;
class UTextBlock;
class UBorder;
class UVerticalBox;
class UEVCheckBoxWidget;
class UEVVocabularyFilterOption;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVocabularyFilterCategorySelectionChanged,
                                            const FEVVocabularyFilterCategorySelection&, Selection);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVVocabularyFilterCategory : public UUserWidget
{
    GENERATED_BODY()

public:
    void ConfigureCategory(EEVVocabularyFilterCategory InCategory, const TArray<EEVVocabularyFilterOption>& InOptions,
                           const TArray<EEVVocabularyFilterOption>& InSelectedOptions);

    void ApplySelection(const FEVVocabularyFilterCategorySelection* Selection);
    const FEVVocabularyFilterCategorySelection& GetSelection() const
    {
        return CurrentSelection;
    }

    UPROPERTY(BlueprintAssignable, Category = "Vocabulary Filters|Events")
    FOnVocabularyFilterCategorySelectionChanged OnSelectionChanged;

protected:
    virtual void NativeOnInitialized() override;

private:
    void PopulateOptions();
    void SetExpanded(bool bInExpanded);
    void RefreshCategoryCheckState();

    UFUNCTION()
    void HandleCategoryButtonPressed();

    UFUNCTION()
    void HandleCategoryCheckStateChanged(bool bIsChecked);

    UFUNCTION()
    void HandleOptionSelectionChanged(EEVVocabularyFilterOption Option, bool bSelected);

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Category = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEVCheckBoxWidget> WBP_CheckBox_Category = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_CategoryName = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> Border_OptionsPanel = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> VerticalBox_Options = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vocabulary Filters|Widget Classes",
              meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UEVVocabularyFilterOption> FilterOptionWidgetClass;

    UPROPERTY(Transient)
    EEVVocabularyFilterCategory Category = EEVVocabularyFilterCategory::None;

    UPROPERTY(Transient)
    TArray<EEVVocabularyFilterOption> AvailableOptions;

    UPROPERTY(Transient)
    FEVVocabularyFilterCategorySelection CurrentSelection;

    UPROPERTY(Transient)
    bool bExpanded = false;

    UPROPERTY(Transient)
    bool bApplyingState = false;
};
