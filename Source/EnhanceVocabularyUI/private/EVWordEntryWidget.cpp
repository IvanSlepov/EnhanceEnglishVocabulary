// Fill out your copyright notice in the Description page of Project Settings.

#include "EVWordEntryWidget.h"
#include "EVEntryItem.h"
#include "Blueprint/WidgetLayoutLibrary.h"

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

    if (Button_ViewWord)
    {
        Button_ViewWord->OnPressed.AddDynamic(this, &ThisClass::HandleOnWordEntry_ViewButtonPressed);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create Button_ViewWord"));
    }
}

void UEVWordEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SizeBoxInitialSize = SizeBox_WordEntry->GetDesiredSize();
}

void UEVWordEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (ListItemObject)
    {
        UEVEntryItem* WordEntryItem = Cast<UEVEntryItem>(ListItemObject);
        if (WordEntryItem && bAreTextFieldsCreated)
        {
            // We are storing the Vocabulary Entry results passed to the widget
            // to broadcast them via the OnWordEntryButtonPressed so that the Player Controller
            // would be able to render the relevant enntry info if user presses a particular's WordEntryWidget View
            // Button
            CurrentWidgetEnryItemToDisplayInPCGeneratedWordEntry.Word = WordEntryItem->EntryItem.Word;
            CurrentWidgetEnryItemToDisplayInPCGeneratedWordEntry.Definition = WordEntryItem->EntryItem.Definition;
            CurrentWidgetEnryItemToDisplayInPCGeneratedWordEntry.Usage = WordEntryItem->EntryItem.Usage;
            CurrentWidgetEnryItemToDisplayInPCGeneratedWordEntry.TranslationRu = WordEntryItem->EntryItem.TranslationRu;
            CurrentWidgetEnryItemToDisplayInPCGeneratedWordEntry.TranslationUa = WordEntryItem->EntryItem.TranslationUa;

            TextBlock_Word_Value->SetText(FText::FromString(WordEntryItem->EntryItem.Word));
            TextBlock_Definition_Value->SetText(FText::FromString(WordEntryItem->EntryItem.Definition));
            TextBlock_Usage_Value->SetText(FText::FromString(WordEntryItem->EntryItem.Usage));
            TextBlock_TranslationRU_Value->SetText(FText::FromString(WordEntryItem->EntryItem.TranslationRu));
            TextBlock_TranslationUA_Value->SetText(FText::FromString(WordEntryItem->EntryItem.TranslationUa));
        }
    }
}

FVocabularyEntry UEVWordEntryWidget::GetCurrentWidgetVocabularyEnryItemInfo()
{
    return CurrentWidgetEnryItemToDisplayInPCGeneratedWordEntry;
}

void UEVWordEntryWidget::HandleOnWordEntry_ViewButtonPressed()
{
    if (bCalledFromPlayerController)
    {
        RemoveFromParent();
    }

    OnWordEntryButtonPressed.Broadcast(this, true, false, false, false);
}

void UEVWordEntryWidget::HandleOnWordEntry_EditButtonPressed()
{
    OnWordEntryButtonPressed.Broadcast(this, false, true, false, false);
}

void UEVWordEntryWidget::HandleOnWordEntry_DeleteButtonPressed()
{
    OnWordEntryButtonPressed.Broadcast(this, false, false, true, false);
}

void UEVWordEntryWidget::HandleOnWordEntry_OkButtonPressed()
{
    OnWordEntryButtonPressed.Broadcast(this, false, false, false, true);
}

void UEVWordEntryWidget::ExpandVisual()
{
    if (SizeBox_WordEntry)
    {
        SizeBox_WordEntry->SetHeightOverride(700.f);
    }
}

void UEVWordEntryWidget::CollapseVisual()
{
    if (SizeBox_WordEntry)
    {
        SizeBox_WordEntry->SetHeightOverride(SizeBoxInitialSize.Y);
    }
}

void UEVWordEntryWidget::ShowWordEntry(const FVocabularyEntry& Entry)
{
    if (bAreTextFieldsCreated)
    {
        ExpandVisual();
        TextBlock_Word_Value->SetText(FText::FromString(Entry.Word));
        TextBlock_Definition_Value->SetText(FText::FromString(Entry.Definition));
        TextBlock_Usage_Value->SetText(FText::FromString(Entry.Usage));
        TextBlock_TranslationRU_Value->SetText(FText::FromString(Entry.TranslationRu));
        TextBlock_TranslationUA_Value->SetText(FText::FromString(Entry.TranslationUa));
    }
}

void UEVWordEntryWidget::SetBoxFill(UHorizontalBox* Box, float FillValue)
{
    if (!Box)
        return;

    UVerticalBoxSlot* VerticalSlot = UWidgetLayoutLibrary::SlotAsVerticalBoxSlot(Box);

    if (!VerticalSlot)
    {
        UE_LOG(LogTemp, Error, TEXT("%s does not have a VerticalBoxSlot"), *Box->GetName());
        return;
    }

    FSlateChildSize ChildSize;
    ChildSize.SizeRule = ESlateSizeRule::Fill;
    ChildSize.Value = FillValue;

    VerticalSlot->SetSize(ChildSize);
}

void UEVWordEntryWidget::SetHorizontalBoxSizeFill()
{

    SetBoxFill(HorizontalBox_Definition);
    SetBoxFill(HorizontalBox_Usage);
    SetBoxFill(HorizontalBox_TranslationUA);
    SetBoxFill(HorizontalBox_TranslationRU);
}

void UEVWordEntryWidget::ViewButtonPressedFromPlayerController()
{
    bCalledFromPlayerController = true;
}
