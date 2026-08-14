// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "EVEntryItem.h"
#include "EVWordEntryActionTypes.h"
#include "EVVocabularyInteractionTypes.h"
#include "EVVocabularyFilterTypes.h"
#include "EVVocabularyLibraryApplicationPort.h"
#include "EVFeatureRoles.h"
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReviewVocabularyValueActionRequested,
                                            const FEVVocabularyValueActionRequest&, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReviewFiltersRequested);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVReviewWordsWidget : public UUserWidget,
                                                     public IEVVocabularyLibraryApplicationPort,
                                                     public IEVReviewFeatureRole,
                                                     public IEVVocabularyPreferencesFeatureRole,
                                                     public IEVWordContextFeatureRole
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UButton> Button_Filter = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_AreFiltersAppliedText = nullptr;

    void DisplayCurrentPage();
    void RefreshReview();
    void SetSearchWord(const FString& Word);
    virtual void ApplyQueryCriteria(const FEVVocabularyQueryCriteria& Criteria) override;

    virtual FOnEVFeatureEntryDetailsRequested& GetEntryDetailsRequestedEvent() override
    {
        return OnEntryDetailsRequested;
    }

    virtual FOnEVFeatureFiltersRequested& GetFiltersRequestedEvent() override
    {
        return OnFeatureFiltersRequested;
    }

    virtual FOnEVFeatureVocabularyValueActionRequested& GetVocabularyValueActionRequestedEvent() override
    {
        return OnFeatureVocabularyValueActionRequested;
    }

    virtual void ApplyVocabularyChange(const FEVVocabularyChangeInfo& ChangeInfo) override;
    virtual void RefreshFeature() override;
    virtual void ApplyVocabularyPreferences(const FEVVocabularyLanguagePreferences& Preferences) override;
    virtual void PresentWordContext(const FString& Word) override;

    virtual FOnEVVocabularyRecordRequested& GetVocabularyRecordRequestedEvent() override
    {
        return OnVocabularyRecordRequested;
    }

    virtual FOnEVVocabularyQueryRequested& GetVocabularyQueryRequestedEvent() override
    {
        return OnVocabularyQueryRequested;
    }

    virtual FOnEVVocabularyMutationRequested& GetVocabularyMutationRequestedEvent() override
    {
        return OnVocabularyMutationRequested;
    }

    virtual void ApplyVocabularyRecordOutcome(const FEVVocabularyRecordOutcome&) override {}
    virtual void ApplyVocabularyQueryOutcome(const FEVVocabularyQueryOutcome& Outcome) override;
    virtual void ApplyVocabularyMutationOutcome(const FEVVocabularyMutationOutcome&) override {}
    virtual void ApplyVocabularyChanged(const FEVVocabularyChangeInfo&) override {}

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

    UPROPERTY(BlueprintAssignable)
    FOnReviewVocabularyValueActionRequested OnVocabularyValueActionRequested;

    UPROPERTY(BlueprintAssignable)
    FOnReviewFiltersRequested OnFiltersRequested;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    UPROPERTY()
    TObjectPtr<UEVWordEntryWidget> CurrentlyExpandedEntryWidget;

    void HandleListEntryWidgetGenerated(UUserWidget& Widget);

    FEVWordEntryActionInfo EVWordEntryActionInfo;

    FOnEVFeatureEntryDetailsRequested OnEntryDetailsRequested;
    FOnEVFeatureFiltersRequested OnFeatureFiltersRequested;
    FOnEVFeatureVocabularyValueActionRequested OnFeatureVocabularyValueActionRequested;

    void HandleEntryDetailsRequested(const FEVVocabularyRecord& Record);

    UFUNCTION()
    void HandleWordEntryViewButtonPressed(UEVWordEntryWidget* CurrentWordEntryWidget);

    UFUNCTION()
    void HandleVocabularyValueActionRequested(const FEVVocabularyValueActionRequest& Request);

    void PopulateEntriesPerPageComboBox();

    UFUNCTION()
    void SetNumberOfEntriesPerPage(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void HandleSearchTextChanged(const FText& NewText);

    UFUNCTION()
    void ClearSearch();

    UFUNCTION()
    void HandleFilterButtonPressed();

    void UpdateFilterStatusText();
    FEVVocabularyRecord BuildFilteredRecordForDisplay(const FEVVocabularyRecord& SourceRecord) const;

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

    UPROPERTY(Transient)
    FEVVocabularyQueryCriteria ActiveQueryCriteria;

    UPROPERTY(Transient)
    FEVVocabularyLanguagePreferences VocabularyPreferences;

    bool bSuppressSearchRefresh = false;

    FReviewPaginationState& GetActivePaginationState();
    const FReviewPaginationState& GetActivePaginationState() const;

    FOnEVVocabularyRecordRequested OnVocabularyRecordRequested;
    FOnEVVocabularyQueryRequested OnVocabularyQueryRequested;
    FOnEVVocabularyMutationRequested OnVocabularyMutationRequested;
    FGuid PendingVocabularyQueryRequestId;
    int32 PendingVocabularyQueryPage = 1;
};
