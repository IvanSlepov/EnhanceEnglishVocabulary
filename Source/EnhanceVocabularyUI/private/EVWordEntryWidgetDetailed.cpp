// Fill out your copyright notice in the Description page of Project Settings.

#include "EVWordEntryWidgetDetailed.h"

void UEVWordEntryWidgetDetailed::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_ViewWord)
    {
        Button_ViewWord->OnPressed.AddDynamic(this, &ThisClass::HandleViewClicked);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_ViewWord is nullptr in EVWordEntryWidgetDetailed.cpp"));
    }
}

void UEVWordEntryWidgetDetailed::NativeConstruct()
{
    Super::NativeConstruct();

    SetEditableFieldsReadOnly(true);
}

void UEVWordEntryWidgetDetailed::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UEVWordEntryWidgetDetailed::ShowWordEntry(const FVocabularyEntry& Entry)
{
    TextBlock_Word_Value->SetText(FText::FromString(Entry.Word));
    MultiLineEditableTextBox_Definition_Value->SetText(FText::FromString(Entry.Definition));
    MultiLineEditableTextBox_Usage_Value->SetText(FText::FromString(Entry.Usage));
    MultiLineEditableTextBox_TranslationRU_Value->SetText(FText::FromString(Entry.TranslationRu));
    MultiLineEditableTextBox_TranslationUA_Value->SetText(FText::FromString(Entry.TranslationUa));
}

void UEVWordEntryWidgetDetailed::SetEditableFieldsReadOnly(bool bSetReadOnly)
{

    MultiLineEditableTextBox_Definition_Value->SetIsReadOnly(bSetReadOnly);
    MultiLineEditableTextBox_Usage_Value->SetIsReadOnly(bSetReadOnly);
    MultiLineEditableTextBox_TranslationUA_Value->SetIsReadOnly(bSetReadOnly);
    MultiLineEditableTextBox_TranslationRU_Value->SetIsReadOnly(bSetReadOnly);
}

void UEVWordEntryWidgetDetailed::HandleViewClicked()
{
    OnViewRequested.Broadcast();
}

void UEVWordEntryWidgetDetailed::HandleEditClicked() {}

void UEVWordEntryWidgetDetailed::HandleDeleteClicked() {}