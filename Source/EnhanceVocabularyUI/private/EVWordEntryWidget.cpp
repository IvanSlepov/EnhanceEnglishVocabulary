// Fill out your copyright notice in the Description page of Project Settings.

#include "EVWordEntryWidget.h"
#include "EVEntryItem.h"
#include "EVVocabularyUiStyle.h"

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

            // We need this adjustment since the long word without interruptions will not be auto-wrapped
            //---start
            const FString& OriginalWord = WordEntryItem->EntryItem.Word;

            TextBlock_Word_Value->SetText(
                FText::FromString(EVVocabularyUiStyle::BuildWrappedWordForDisplay(OriginalWord)));

            // here we set a tooltip containing the whole world
            TextBlock_Word_Value->SetToolTipText(FText::FromString(OriginalWord));
            // --end

            TextBlock_Definition_Value->SetText(FText::FromString(WordEntryItem->EntryItem.Definition));
            TextBlock_Usage_Value->SetText(FText::FromString(WordEntryItem->EntryItem.Usage));

            // Set the Usage color to Red if no usage was provided
            TextBlock_Usage_Value->SetColorAndOpacity(WordEntryItem->EntryItem.bHasUsageExamples
                                                          ? EVVocabularyUiStyle::GetNormalWrodEntryTextFontColor()
                                                          : EVVocabularyUiStyle::GetMissingWrodEntryTextFontColor());

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
    OnWordEntryViewButtonPressed.Broadcast(this);
}
