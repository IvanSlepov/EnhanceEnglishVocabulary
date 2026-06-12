// Fill out your copyright notice in the Description page of Project Settings.

#include "EVWordEntryWidget.h"
#include "EVEntryItem.h"

void UEVWordEntryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (TextBlock_Word_Value && TextBlock_Definition_Value && TextBlock_Usage_Value && TextBlock_TranslationUA_Value &&
        TextBlock_TranslationRU_Value)
    {
        bAreTextFieldsCreated = true;
    }
    else
    {
        bAreTextFieldsCreated = false;
    }
}

void UEVWordEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UEVWordEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (ListItemObject)
    {
        UEVEntryItem* WordEntryItem = Cast<UEVEntryItem>(ListItemObject);
        if (WordEntryItem && bAreTextFieldsCreated)
        {
            TextBlock_Word_Value->SetText(FText::FromString(WordEntryItem->EntryItem.Word));
            TextBlock_Definition_Value->SetText(FText::FromString(WordEntryItem->EntryItem.Definition));
            TextBlock_Usage_Value->SetText(FText::FromString(WordEntryItem->EntryItem.Usage));
            TextBlock_TranslationRU_Value->SetText(FText::FromString(WordEntryItem->EntryItem.TranslationRu));
            TextBlock_TranslationUA_Value->SetText(FText::FromString(WordEntryItem->EntryItem.TranslationUa));
        }
    }
}
