// Fill out your copyright notice in the Description page of Project Settings.

#include "EVReviewWordsWidget.h"

#include "EnhanceVocabulary/EVGameInstance.h"
#include "EnhanceVocabularyCore/public/EVVocabularyTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EVWordEntryWidget.h"

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
}

void UEVReviewWordsWidget::NativePreConstruct() {}

void UEVReviewWordsWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UEVReviewWordsWidget::DisplayWords()
{
    if (!EVGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to cast to EVGameInstance"));
        return;
    }

    if (!ListView_ReviewWords)
    {
        UE_LOG(LogTemp, Error, TEXT("Missing ListView_ReviewWords"));
        return;
    }

    ListView_ReviewWords->ClearListItems();

    TArray<FVocabularyEntry> VocabularyEntries;
    EVGameInstance->GetVocabularyEntries(VocabularyEntries, NumberOfWordsToDisplay);

    if (VocabularyEntries.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Vocabulary is empty!!!"));
    }

    for (auto Entry : VocabularyEntries)
    {
        UEVEntryItem* EVEntryItem = NewObject<UEVEntryItem>(this, UEVEntryItem::StaticClass());
        if (EVEntryItem)
        {
            EVEntryItem->EntryItem = Entry;

            ListView_ReviewWords->AddItem(EVEntryItem);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create the EVEntryItem object"));
        }
    }
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
