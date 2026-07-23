// Fill out your copyright notice in the Description page of Project Settings.

#include "EVWordEntryWidgetDetailed.h"

#include "EVVocabularyUiStyle.h"

void UEVWordEntryWidgetDetailed::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_ViewWord)
    {
        Button_ViewWord->OnPressed.AddDynamic(this, &ThisClass::HandleViewPressed);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_ViewWord is nullptr in EVWordEntryWidgetDetailed.cpp"));
    }

    if (Button_EditWordEntry)
    {
        Button_EditWordEntry->OnPressed.AddDynamic(this, &ThisClass::HandleEditPressed);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_EditWordEntry is nullptr in EVWordEntryWidgetDetailed.cpp"));
    }

    if (Button_SaveChanges)
    {
        Button_SaveChanges->OnPressed.AddDynamic(this, &ThisClass::HandleSaveChangesPressed);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_SaveChanges is nullptr in EVWordEntryWidgetDetailed.cpp"));
    }

    if (Button_DeleteWordEntry)
    {
        Button_DeleteWordEntry->OnPressed.AddDynamic(this, &ThisClass::HandleDeletePressed);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_DeleteWordEntry is nullptr in EVWordEntryWidgetDetailed.cpp"));
    }
}

void UEVWordEntryWidgetDetailed::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UEVWordEntryWidgetDetailed::NativeConstruct()
{
    Super::NativeConstruct();

    SetEditableFieldsReadOnly(true);
    Button_SaveChanges->SetVisibility(ESlateVisibility::Hidden);
}

void UEVWordEntryWidgetDetailed::ShowWordEntry(const FVocabularyEntry& Entry)
{
    // same reason as in the #include "EVWordEntryWidget.cpp"
    // ---start
    CurrentOriginalWord = Entry.Word;

    TextBlock_Word_Value->SetText(
        FText::FromString(EVVocabularyUiStyle::BuildWrappedWordForDisplay(CurrentOriginalWord)));

    TextBlock_Word_Value->SetToolTipText(FText::FromString(CurrentOriginalWord));
    // ---end

    MultiLineEditableTextBox_Transcription_Value->SetText(FText::FromString(Entry.Transcription));
    MultiLineEditableTextBox_Definition_Value->SetText(FText::FromString(Entry.Definition));
    MultiLineEditableTextBox_Usage_Value->SetText(FText::FromString(Entry.Usage));
    MultiLineEditableTextBox_TranslationRU_Value->SetText(FText::FromString(Entry.TranslationRu));
    MultiLineEditableTextBox_TranslationUA_Value->SetText(FText::FromString(Entry.TranslationUa));
}

void UEVWordEntryWidgetDetailed::SetButtonsDisabled(bool bIsViewButtonDisabled, bool bIsEditButtonDisabled,
                                                    bool bIsDeleteButtonDisabled, bool bIsSaveChangesButtonHidden)
{
    if (bIsViewButtonDisabled)
    {
        if (Button_ViewWord->GetIsEnabled())
        {
            Button_ViewWord->SetIsEnabled(!bIsViewButtonDisabled);
        }
    }
    else
    {
        if (!Button_ViewWord->GetIsEnabled())
        {
            Button_ViewWord->SetIsEnabled(!bIsViewButtonDisabled);
        }
    }

    if (bIsEditButtonDisabled)
    {
        if (Button_EditWordEntry->GetIsEnabled())
        {
            Button_EditWordEntry->SetIsEnabled(!bIsEditButtonDisabled);
        }
    }
    else
    {
        if (!Button_EditWordEntry->GetIsEnabled())
        {
            Button_EditWordEntry->SetIsEnabled(!bIsEditButtonDisabled);
        }
    }

    if (bIsDeleteButtonDisabled)
    {
        if (Button_DeleteWordEntry->GetIsEnabled())
        {
            Button_DeleteWordEntry->SetIsEnabled(!bIsDeleteButtonDisabled);
        }
    }
    else
    {
        if (!Button_DeleteWordEntry->GetIsEnabled())
        {
            Button_DeleteWordEntry->SetIsEnabled(!bIsDeleteButtonDisabled);
        }
    }

    if (bIsSaveChangesButtonHidden)
    {
        if (Button_SaveChanges->IsVisible())
        {
            Button_SaveChanges->SetVisibility(ESlateVisibility::Hidden);
        }
    }
    else
    {
        if (!Button_SaveChanges->IsVisible())
        {
            Button_SaveChanges->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

void UEVWordEntryWidgetDetailed::SetEditableFieldsReadOnly(bool bSetReadOnly)
{

    MultiLineEditableTextBox_Transcription_Value->SetIsReadOnly(bSetReadOnly);
    MultiLineEditableTextBox_Definition_Value->SetIsReadOnly(bSetReadOnly);
    MultiLineEditableTextBox_Usage_Value->SetIsReadOnly(bSetReadOnly);
    MultiLineEditableTextBox_TranslationUA_Value->SetIsReadOnly(bSetReadOnly);
    MultiLineEditableTextBox_TranslationRU_Value->SetIsReadOnly(bSetReadOnly);
}

void UEVWordEntryWidgetDetailed::HandleViewPressed()
{
    OnViewRequested.Broadcast();
}

void UEVWordEntryWidgetDetailed::HandleEditPressed()
{
    OnEditRequested.Broadcast();
}

void UEVWordEntryWidgetDetailed::HandleSaveChangesPressed()
{
    FVocabularyEntry EditedVocabularyEntry;

    EditedVocabularyEntry.Word = CurrentOriginalWord;
    EditedVocabularyEntry.Transcription = MultiLineEditableTextBox_Transcription_Value->GetText().ToString();
    EditedVocabularyEntry.Definition = MultiLineEditableTextBox_Definition_Value->GetText().ToString();
    EditedVocabularyEntry.Usage = MultiLineEditableTextBox_Usage_Value->GetText().ToString();
    EditedVocabularyEntry.TranslationRu = MultiLineEditableTextBox_TranslationRU_Value->GetText().ToString();
    EditedVocabularyEntry.TranslationUa = MultiLineEditableTextBox_TranslationUA_Value->GetText().ToString();

    OnWordEntryChangesSubmitted.Broadcast(EditedVocabularyEntry);
}

void UEVWordEntryWidgetDetailed::HandleDeletePressed()
{
    OnDeleteRequested.Broadcast();
}