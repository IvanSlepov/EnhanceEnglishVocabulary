// Fill out your copyright notice in the Description page of Project Settings.

#include "EVReviewWordsWidget.h"

#include "EnhanceVocabularyCore/public/EVVocabularyTypes.h"
#include "EVVocabularyLegacyCompatibility.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EVWordEntryWidget.h"
#include "EVWordInputValidator.h"
#include "EVVocabularyUiStyle.h"
#include "EVVocabularyFilterTypes.h"
#include "Widgets/Views/STableViewBase.h"

// Amount of the entries we can display per page
const TArray<int32> UEVReviewWordsWidget::SupportedEntriesPerPageValues = {5, 10, 25, 50};

void UEVReviewWordsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (ListView_ReviewWords)
    {
        ListView_ReviewWords->OnEntryWidgetGenerated().AddUObject(
            this, &UEVReviewWordsWidget::HandleListEntryWidgetGenerated);
    }

    if (Button_PreviousPage)
    {
        Button_PreviousPage->OnPressed.AddUniqueDynamic(this, &ThisClass::GoToPreviousPage);
    }

    if (Button_NextPage)
    {
        Button_NextPage->OnPressed.AddUniqueDynamic(this, &ThisClass::GoToNextPage);
    }

    if (ComboBoxString_EntriesPerPage)
    {
        ComboBoxString_EntriesPerPage->OnSelectionChanged.AddDynamic(this, &ThisClass::SetNumberOfEntriesPerPage);
    }

    if (EditableTextBox_Search)
    {
        EditableTextBox_Search->OnTextChanged.AddDynamic(this, &ThisClass::HandleSearchTextChanged);
    }

    if (Button_ClearSearch)
    {
        Button_ClearSearch->OnPressed.AddUniqueDynamic(this, &ThisClass::ClearSearch);
    }

    if (Button_Filter)
    {
        Button_Filter->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleFilterButtonPressed);
    }
}

void UEVReviewWordsWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UEVReviewWordsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ListView_ReviewWords)
    {
        ListView_ReviewWords->SetScrollIntoViewAlignment(EScrollIntoViewAlignment::TopOrLeft);
    }

    NormalPaginationState.CurrentPage = CurrentPage;
    NormalPaginationState.EntriesPerPage = EntriesPerPage;

    SearchPaginationState.CurrentPage = 1;
    SearchPaginationState.EntriesPerPage = DefaultEntriesPerPage;

    UpdateFilterStatusText();
    RefreshReview();
    PopulateEntriesPerPageComboBox();
}

void UEVReviewWordsWidget::SetSearchWord(const FString& Word)
{
    if (!EditableTextBox_Search)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot set notification word search: search box is null."));
        return;
    }

    SearchPaginationState.CurrentPage = 1;
    bSuppressSearchRefresh = true;
    EditableTextBox_Search->SetText(FText::FromString(Word));
    bSuppressSearchRefresh = false;
    RefreshReview();
}

void UEVReviewWordsWidget::DisplayCurrentPage()
{
    if (!ListView_ReviewWords)
    {
        return;
    }

    const int32 Offset = (CurrentPage - 1) * EntriesPerPage;

    FString SearchPrefix;
    FText SearchError;

    const bool bSearchEmpty = IsSearchInputEmpty();
    const bool bSearchValid = !bSearchEmpty && TryGetValidatedSearchInput(SearchPrefix, SearchError);

    if (!bSearchEmpty && !bSearchValid)
    {
        ListView_ReviewWords->ClearListItems();
        return;
    }

    if (!OnVocabularyQueryRequested.IsBound())
    {
        return;
    }

    FEVVocabularyQueryRequest Request;
    Request.RequestId = FGuid::NewGuid();
    Request.SearchPrefix = bSearchValid ? SearchPrefix : FString();
    Request.Criteria = ActiveQueryCriteria;
    Request.Limit = EntriesPerPage;
    Request.Offset = Offset;

    PendingVocabularyQueryRequestId = Request.RequestId;
    PendingVocabularyQueryPage = CurrentPage;
    OnVocabularyQueryRequested.Broadcast(Request);
}
void UEVReviewWordsWidget::ApplyVocabularyQueryOutcome(const FEVVocabularyQueryOutcome& Outcome)
{
    if (!PendingVocabularyQueryRequestId.IsValid() || Outcome.RequestId != PendingVocabularyQueryRequestId)
    {
        return;
    }

    const int32 RequestedPage = PendingVocabularyQueryPage;
    PendingVocabularyQueryRequestId = FGuid();

    TotalEntries = FMath::Max(Outcome.TotalEntries, 0);
    TotalPages = TotalEntries > 0 ? (TotalEntries + EntriesPerPage - 1) / EntriesPerPage : 1;
    CurrentPage = FMath::Clamp(CurrentPage, 1, TotalPages);

    FReviewPaginationState& ActivePaginationState = GetActivePaginationState();
    ActivePaginationState.CurrentPage = CurrentPage;
    ActivePaginationState.EntriesPerPage = EntriesPerPage;

    if (CurrentPage != RequestedPage)
    {
        DisplayCurrentPage();
        return;
    }

    if (Outcome.Result != EEVApplicationOperationResult::Succeeded)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load vocabulary page %d."), CurrentPage);
        UpdatePaginationControls();
        return;
    }

    if (!ListView_ReviewWords)
    {
        return;
    }

    ListView_ReviewWords->ClearListItems();

    int32 AddedRecordCount = 0;

    for (const FEVVocabularyRecord& VocabularyRecord : Outcome.Records)
    {
        UEVEntryItem* EntryItem = NewObject<UEVEntryItem>(this);

        if (!EntryItem)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create review entry item."));
            continue;
        }

        EntryItem->PayloadType = EEVEntryItemPayloadType::VocabularyRecord;
        EntryItem->VocabularyRecord = VocabularyRecord;
        ListView_ReviewWords->AddItem(EntryItem);
        ++AddedRecordCount;
    }

    if (AddedRecordCount > 0)
    {
        ListView_ReviewWords->SetScrollIntoViewAlignment(EScrollIntoViewAlignment::TopOrLeft);
        ListView_ReviewWords->ScrollToTop();
        ListView_ReviewWords->ScrollIndexIntoView(0);
    }

    UpdatePaginationControls();
}

void UEVReviewWordsWidget::RefreshReview()
{
    if (!ListView_ReviewWords)
    {
        return;
    }

    FReviewPaginationState& ActivePaginationState = GetActivePaginationState();

    CurrentPage = ActivePaginationState.CurrentPage;
    EntriesPerPage = ActivePaginationState.EntriesPerPage;

    if (ComboBoxString_EntriesPerPage)
    {
        ComboBoxString_EntriesPerPage->SetSelectedOption(FString::FromInt(EntriesPerPage));
    }

    FString SearchPrefix;
    FText SearchError;

    const bool bSearchEmpty = IsSearchInputEmpty();
    const bool bSearchValid = !bSearchEmpty && TryGetValidatedSearchInput(SearchPrefix, SearchError);

    if (!bSearchEmpty && !bSearchValid)
    {
        return;
    }

    if (!SupportedEntriesPerPageValues.Contains(EntriesPerPage))
    {
        UE_LOG(LogTemp, Warning, TEXT("Unsupported EntriesPerPage value: %d. Falling back to default (%d)."),
               EntriesPerPage, DefaultEntriesPerPage);

        EntriesPerPage = DefaultEntriesPerPage;
    }

    ActivePaginationState.CurrentPage = CurrentPage;
    ActivePaginationState.EntriesPerPage = EntriesPerPage;

    DisplayCurrentPage();
}

void UEVReviewWordsWidget::HandleListEntryWidgetGenerated(UUserWidget& Widget)
{
    UEVWordEntryWidget* WordEntryWidget = Cast<UEVWordEntryWidget>(&Widget);

    if (!WordEntryWidget)
    {
        return;
    }
    /*Why use AddUniqueDynamic?*/
    /*ListView virtualizes its items. Instead of creating a new widget for every row forever, it reuses existing entry
       widgets as you scroll or refresh the list.

        Every time a widget is (re)assigned to a list item, OnEntryWidgetGenerated is called again. Previously the code
       did:

        WordEntryWidget->OnWordEntryViewButtonPressed.AddDynamic(...);

        each time,
        so the same handler was being added to the same delegate repeatedly.

            Dynamic multicast delegates in Unreal do not allow the exact same object /
            function pair to be bound more than once,

        so Unreal raised an ensure to warn about the duplicate binding.

        AddUniqueDynamic first checks whether that binding already exists :

        If it doesn't, it adds it. If it does,
        it does nothing.

        So each UEVWordEntryWidget ends up with exactly one binding,
        even though the ListView generates or recycles it multiple times*/

    WordEntryWidget->ApplyPresentationContext(ActiveQueryCriteria, VocabularyPreferences);
    WordEntryWidget->GetEntryDetailsRequestedEvent().RemoveAll(this);
    WordEntryWidget->GetEntryDetailsRequestedEvent().AddUObject(this, &ThisClass::HandleEntryDetailsRequested);
    WordEntryWidget->OnValueActionRequested.AddUniqueDynamic(this, &ThisClass::HandleVocabularyValueActionRequested);
}

void UEVReviewWordsWidget::HandleEntryDetailsRequested(const FEVVocabularyRecord& Record)
{
    OnEntryDetailsRequested.Broadcast(Record);

    EVWordEntryActionInfo.ActionType = EEVWordEntryActionType::ViewEntry;
    EVWordEntryActionInfo.EntryInfo = EVVocabularyLegacyCompatibility::FlattenRecord(Record);
    OnWordEntryWidgetControlsButtonPressed.Broadcast(EVWordEntryActionInfo);
}

void UEVReviewWordsWidget::HandleWordEntryViewButtonPressed(UEVWordEntryWidget* CurrentWordEntryWidget)
{
    if (!CurrentWordEntryWidget)
    {
        UE_LOG(LogTemp, Error,
               TEXT("The UEVWordEntryWidget CurrentWordEntryWidget is nullptr in EVReviewWordsWidget.cpp"));
        return;
    }

    HandleEntryDetailsRequested(CurrentWordEntryWidget->GetCurrentVocabularyRecord());
}

void UEVReviewWordsWidget::PopulateEntriesPerPageComboBox()
{
    if (!ComboBoxString_EntriesPerPage)
    {
        UE_LOG(LogTemp, Error, TEXT("ComboBoxString_EntriesPerPage is nullptr in EVReviewWordsWidget.cpp"));
        return;
    }

    ComboBoxString_EntriesPerPage->ClearOptions();

    for (int32 NumberOfEntry : SupportedEntriesPerPageValues)
    {
        FString NumberOfEntriesToString = FString::FromInt(NumberOfEntry);
        ComboBoxString_EntriesPerPage->AddOption(NumberOfEntriesToString);
    }

    ComboBoxString_EntriesPerPage->SetSelectedOption(FString::FromInt(EntriesPerPage));
}

void UEVReviewWordsWidget::SetNumberOfEntriesPerPage(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    const int32 SelectedEntriesPerPage = FCString::Atoi(*SelectedItem);

    if (!SupportedEntriesPerPageValues.Contains(SelectedEntriesPerPage))
    {
        UE_LOG(LogTemp, Warning, TEXT("Unsupported entries-per-page value: %s"), *SelectedItem);

        return;
    }

    FReviewPaginationState& ActivePaginationState = GetActivePaginationState();

    if (ActivePaginationState.EntriesPerPage == SelectedEntriesPerPage)
    {
        return;
    }

    ActivePaginationState.EntriesPerPage = SelectedEntriesPerPage;
    ActivePaginationState.CurrentPage = 1;

    RefreshReview();
}

void UEVReviewWordsWidget::HandleSearchTextChanged(const FText& NewText)
{
    if (bSuppressSearchRefresh)
    {
        return;
    }

    RefreshReview();
}

void UEVReviewWordsWidget::ClearSearch()
{
    if (!EditableTextBox_Search)
    {
        return;
    }

    bSuppressSearchRefresh = true;
    EditableTextBox_Search->SetText(FText::GetEmpty());
    bSuppressSearchRefresh = false;
    RefreshReview();
}

bool UEVReviewWordsWidget::TryGetValidatedSearchInput(FString& OutNormalizedSearch, FText& OutErrorMessage) const
{
    OutNormalizedSearch.Empty();
    OutErrorMessage = FText::GetEmpty();

    if (!EditableTextBox_Search)
    {
        return false;
    }

    const FString RawInput = EditableTextBox_Search->GetText().ToString();

    const EEVInputValidationResult ValidationResult =
        FEVWordInputValidator::ValidateSearchInput(RawInput, OutNormalizedSearch, OutErrorMessage);

    return ValidationResult == EEVInputValidationResult::Valid;
}

bool UEVReviewWordsWidget::IsSearchInputEmpty() const
{
    if (!EditableTextBox_Search)
    {
        return true;
    }

    return EditableTextBox_Search->GetText().ToString().TrimStartAndEnd().IsEmpty();
}

UEVReviewWordsWidget::FReviewPaginationState& UEVReviewWordsWidget::GetActivePaginationState()
{
    return IsSearchInputEmpty() ? NormalPaginationState : SearchPaginationState;
}

const UEVReviewWordsWidget::FReviewPaginationState& UEVReviewWordsWidget::GetActivePaginationState() const
{
    return IsSearchInputEmpty() ? NormalPaginationState : SearchPaginationState;
}

void UEVReviewWordsWidget::UpdateDisplayedWordEntry(const FVocabularyEntry& UpdatedEntry)
{
    UE_LOG(LogTemp, Log, TEXT("Refreshing review after updating word: %s"), *UpdatedEntry.Word);

    RefreshReview();
}

void UEVReviewWordsWidget::RemoveDisplayedWordEntry(const FVocabularyEntry& DeletedEntry)
{
    UE_LOG(LogTemp, Log, TEXT("Refreshing review after deleting word: %s"), *DeletedEntry.Word);

    RefreshReview();
}

void UEVReviewWordsWidget::UpdatePaginationControls()
{
    if (Text_CurrentPage)
    {
        Text_CurrentPage->SetText(FText::Format(NSLOCTEXT("EVReviewWordsWidget", "PageIndicator", "Page {0} of {1}"),
                                                FText::AsNumber(CurrentPage), FText::AsNumber(TotalPages)));
    }

    if (Button_PreviousPage)
    {
        Button_PreviousPage->SetIsEnabled(CurrentPage > 1);
    }

    if (Button_NextPage)
    {
        Button_NextPage->SetIsEnabled(CurrentPage < TotalPages);
    }
}

void UEVReviewWordsWidget::GoToPage(int32 PageNumber)
{
    FReviewPaginationState& ActivePaginationState = GetActivePaginationState();

    const int32 ClampedPage = FMath::Clamp(PageNumber, 1, TotalPages);

    if (ClampedPage == ActivePaginationState.CurrentPage)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Changing review page: %d -> %d"), ActivePaginationState.CurrentPage, ClampedPage);

    ActivePaginationState.CurrentPage = ClampedPage;

    RefreshReview();
}

void UEVReviewWordsWidget::GoToNextPage()
{
    const FReviewPaginationState& ActivePaginationState = GetActivePaginationState();

    GoToPage(ActivePaginationState.CurrentPage + 1);
}

void UEVReviewWordsWidget::GoToPreviousPage()
{
    const FReviewPaginationState& ActivePaginationState = GetActivePaginationState();

    GoToPage(ActivePaginationState.CurrentPage - 1);
}
void UEVReviewWordsWidget::HandleVocabularyValueActionRequested(const FEVVocabularyValueActionRequest& Request)
{
    OnVocabularyValueActionRequested.Broadcast(Request);
    OnFeatureVocabularyValueActionRequested.Broadcast(Request);
}

void UEVReviewWordsWidget::ApplyQueryCriteria(const FEVVocabularyQueryCriteria& Criteria)
{
    ActiveQueryCriteria = Criteria;
    ActiveQueryCriteria.Normalize();
    NormalPaginationState.CurrentPage = 1;
    SearchPaginationState.CurrentPage = 1;
    UpdateFilterStatusText();
    ClearSearch();
}

void UEVReviewWordsWidget::HandleFilterButtonPressed()
{
    OnFiltersRequested.Broadcast();
    OnFeatureFiltersRequested.Broadcast();
}

void UEVReviewWordsWidget::ApplyVocabularyChange(const FEVVocabularyChangeInfo&)
{
    RefreshReview();
}

void UEVReviewWordsWidget::RefreshFeature()
{
    RefreshReview();
}

void UEVReviewWordsWidget::ApplyVocabularyPreferences(const FEVVocabularyLanguagePreferences& Preferences)
{
    VocabularyPreferences = Preferences;
    VocabularyPreferences.Normalize();
    RefreshReview();
}

void UEVReviewWordsWidget::PresentWordContext(const FString& Word)
{
    SetSearchWord(Word);
}

void UEVReviewWordsWidget::UpdateFilterStatusText()
{
    if (!TextBlock_AreFiltersAppliedText)
    {
        return;
    }

    const bool bHasFilters = ActiveQueryCriteria.HasActiveFilters();
    TextBlock_AreFiltersAppliedText->SetText(
        bHasFilters ? NSLOCTEXT("EVReviewWordsWidget", "ActiveFilters", "ACTIVE FILTER/S")
                    : NSLOCTEXT("EVReviewWordsWidget", "NoActiveFilters", "NO ACTIVE FILTERS"));
    TextBlock_AreFiltersAppliedText->SetColorAndOpacity(bHasFilters
                                                            ? EVVocabularyUiStyle::GetActiveFiltersTextFontColor()
                                                            : EVVocabularyUiStyle::GetInactiveFiltersTextFontColor());
}

FEVVocabularyRecord UEVReviewWordsWidget::BuildFilteredRecordForDisplay(const FEVVocabularyRecord& SourceRecord) const
{
    if (!ActiveQueryCriteria.HasActiveFilters())
    {
        return SourceRecord;
    }

    FEVVocabularyRecord FilteredRecord = SourceRecord;
    FilteredRecord.Meanings.RemoveAll(
        [this](const FEVVocabularyMeaning& Meaning)
        { return !EVVocabularyFilter::MeaningMatchesCriteria(Meaning, ActiveQueryCriteria); });
    return FilteredRecord;
}
