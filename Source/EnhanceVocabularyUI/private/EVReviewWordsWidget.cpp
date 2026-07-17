// Fill out your copyright notice in the Description page of Project Settings.

#include "EVReviewWordsWidget.h"

#include "EnhanceVocabulary/EVGameInstance.h"
#include "EnhanceVocabularyCore/public/EVVocabularyTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EVWordEntryWidget.h"

// Amount of the entries we can display per page
const TArray<int32> UEVReviewWordsWidget::SupportedEntriesPerPageValues = {5, 10, 25, 50};

void UEVReviewWordsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    EVGameInstance = Cast<UEVGameInstance>(GetGameInstance());
    if (!EVGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to cast to EVGameInstance"));
    }

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
}

void UEVReviewWordsWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UEVReviewWordsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    RefreshReview();
    PopulateEntriesPerPageComboBox();
}

void UEVReviewWordsWidget::DisplayCurrentPage()
{
    if (!EVGameInstance || !ListView_ReviewWords)
    {
        return;
    }

    const int32 Offset = (CurrentPage - 1) * EntriesPerPage;

    TArray<FVocabularyEntry> VocabularyEntries;

    const bool bLoadedSuccessfully =
        EVGameInstance->GetVocabularyEntriesPage(VocabularyEntries, EntriesPerPage, Offset);

    if (!bLoadedSuccessfully)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load vocabulary page %d."), CurrentPage);

        return;
    }

    ListView_ReviewWords->ClearListItems();

    for (const FVocabularyEntry& Entry : VocabularyEntries)
    {
        UEVEntryItem* EntryItem = NewObject<UEVEntryItem>(this);

        if (!EntryItem)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create review entry item."));
            continue;
        }

        EntryItem->EntryItem = Entry;
        ListView_ReviewWords->AddItem(EntryItem);
    }

    ListView_ReviewWords->ScrollToTop();
}

void UEVReviewWordsWidget::RefreshReview()
{
    if (!EVGameInstance || !ListView_ReviewWords)
    {
        return;
    }

    TotalEntries = EVGameInstance->GetVocabularyEntryCount();

    if (!SupportedEntriesPerPageValues.Contains(EntriesPerPage))
    {
        UE_LOG(LogTemp, Warning, TEXT("Unsupported EntriesPerPage value: %d. Falling back to default (%d)."),
               EntriesPerPage, DefaultEntriesPerPage);

        EntriesPerPage = DefaultEntriesPerPage;
    }

    TotalPages = TotalEntries > 0 ? (TotalEntries + EntriesPerPage - 1) / EntriesPerPage : 1;

    CurrentPage = FMath::Clamp(CurrentPage, 1, TotalPages);

    DisplayCurrentPage();
    UpdatePaginationControls();
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

    WordEntryWidget->OnWordEntryViewButtonPressed.AddUniqueDynamic(this, &ThisClass::HandleWordEntryViewButtonPressed);
}

void UEVReviewWordsWidget::HandleWordEntryViewButtonPressed(UEVWordEntryWidget* CurrentWordEntryWidget)
{
    if (!CurrentWordEntryWidget)
    {
        UE_LOG(LogTemp, Error,
               TEXT("The UEVWordEntryWidget CurrentWordEntryWidget is nullptr in EVReviewWordsWidget.cpp"));
        return;
    }

    EVWordEntryActionInfo.ActionType = EEVWordEntryActionType::ViewEntry;
    EVWordEntryActionInfo.EntryInfo = CurrentWordEntryWidget->GetCurrentWidgetVocabularyEnryItemInfo();

    OnWordEntryWidgetControlsButtonPressed.Broadcast(EVWordEntryActionInfo);
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

    if (EntriesPerPage == SelectedEntriesPerPage)
    {
        return;
    }

    EntriesPerPage = SelectedEntriesPerPage;

    // Usually best UX when page size changes.
    CurrentPage = 1;

    RefreshReview();
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
    const int32 ClampedPage = FMath::Clamp(PageNumber, 1, TotalPages);

    if (ClampedPage == CurrentPage)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Changing review page: %d -> %d"), CurrentPage, ClampedPage);

    CurrentPage = ClampedPage;

    DisplayCurrentPage();
    UpdatePaginationControls();
}

void UEVReviewWordsWidget::GoToNextPage()
{
    GoToPage(CurrentPage + 1);
}

void UEVReviewWordsWidget::GoToPreviousPage()
{
    GoToPage(CurrentPage - 1);
}