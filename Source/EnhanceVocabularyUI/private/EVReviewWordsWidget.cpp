// Fill out your copyright notice in the Description page of Project Settings.

#include "EVReviewWordsWidget.h"

#include "EnhanceVocabulary/EVGameInstance.h"
#include "EnhanceVocabularyCore/public/EVVocabularyTypes.h"
#include "EnhanceVocabularyStorage/public/EVEntryItem.h"
#include "Kismet/KismetSystemLibrary.h"

void UEVReviewWordsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    EVGameInstance = Cast<UEVGameInstance>(GetGameInstance());
    if (!EVGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to cast to EVGameInstance"));
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

    ListView_ReviewWords->ClearListItems();

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