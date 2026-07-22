// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "EVEntryItem.h"
#include "EVWordEntryActionTypes.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "EVReviewWordsWidget.generated.h"

class UEVWordEntryWidget;
class UUserWidget;

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWordEntryWidgetControlsButtonPressed, const FEVWordEntryActionInfo&,
                                            WordEntryActionInfo);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVReviewWordsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UListView* ListView_ReviewWords;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_PreviousPage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_NextPage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* Text_CurrentPage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UComboBoxString* ComboBoxString_EntriesPerPage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UEditableTextBox* EditableTextBox_Search;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_ClearSearch;

    class UEVGameInstance* EVGameInstance;

    void DisplayCurrentPage();
    void RefreshReview();

    void UpdateDisplayedWordEntry(const FVocabularyEntry& UpdatedEntry);
    void RemoveDisplayedWordEntry(const FVocabularyEntry& DeletedEntry);
    void UpdatePaginationControls();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pagination")
    int32 EntriesPerPage = 10;

    UPROPERTY(BlueprintReadOnly, Category = "Pagination")
    int32 CurrentPage = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Pagination")
    int32 TotalEntries = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Pagination")
    int32 TotalPages = 1;

    UFUNCTION(BlueprintCallable, Category = "Pagination")
    void GoToPage(int32 PageNumber);

    UFUNCTION(BlueprintCallable, Category = "Pagination")
    void GoToNextPage();

    UFUNCTION(BlueprintCallable, Category = "Pagination")
    void GoToPreviousPage();

    UPROPERTY(BlueprintAssignable)
    FOnWordEntryWidgetControlsButtonPressed OnWordEntryWidgetControlsButtonPressed;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    UPROPERTY()
    TObjectPtr<UEVWordEntryWidget> CurrentlyExpandedEntryWidget;

    void HandleListEntryWidgetGenerated(UUserWidget& Widget);

    FEVWordEntryActionInfo EVWordEntryActionInfo;

    UFUNCTION()
    void HandleWordEntryViewButtonPressed(UEVWordEntryWidget* CurrentWordEntryWidget);

    void PopulateEntriesPerPageComboBox();

    UFUNCTION()
    void SetNumberOfEntriesPerPage(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void HandleSearchTextChanged(const FText& NewText);

    UFUNCTION()
    void ClearSearch();

    static constexpr int32 DefaultEntriesPerPage = 10;

    static const TArray<int32> SupportedEntriesPerPageValues;

    bool TryGetValidatedSearchInput(FString& OutNormalizedSearch, FText& OutErrorMessage) const;

    bool IsSearchInputEmpty() const;

    struct FReviewPaginationState
    {
        int32 CurrentPage = 1;
        int32 EntriesPerPage = 10;
    };

    FReviewPaginationState NormalPaginationState;
    FReviewPaginationState SearchPaginationState;

    FReviewPaginationState& GetActivePaginationState();
    const FReviewPaginationState& GetActivePaginationState() const;
};
