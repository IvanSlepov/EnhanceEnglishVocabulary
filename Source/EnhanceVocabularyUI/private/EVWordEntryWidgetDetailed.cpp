// Fill out your copyright notice in the Description page of Project Settings.

#include "EVWordEntryWidgetDetailed.h"

#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "EVEntryItem.h"
#include "EVVocabularyEntryMeaningWidget.h"
#include "EVVocabularyTranslationUtils.h"
#include "EVVocabularyUiStyle.h"
#include "EVWordInputValidator.h"

void UEVWordEntryWidgetDetailed::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_ViewWord)
    {
        Button_ViewWord->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleViewPressed);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_ViewWord is nullptr in EVWordEntryWidgetDetailed.cpp"));
    }

    if (Button_EditWordEntry)
    {
        Button_EditWordEntry->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleEditPressed);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_EditWordEntry is nullptr in EVWordEntryWidgetDetailed.cpp"));
    }

    if (Button_SaveChanges)
    {
        Button_SaveChanges->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleSaveChangesPressed);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_SaveChanges is nullptr in EVWordEntryWidgetDetailed.cpp"));
    }

    if (Button_DeleteWordEntry)
    {
        Button_DeleteWordEntry->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleDeletePressed);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_DeleteWordEntry is nullptr in EVWordEntryWidgetDetailed.cpp"));
    }

    if (ListView_MeaningWidgets)
    {
        ListView_MeaningWidgets->OnEntryWidgetGenerated().AddUObject(this,
                                                                     &ThisClass::HandleMeaningEntryWidgetGenerated);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ListView_MeaningWidgets is nullptr in EVWordEntryWidgetDetailed.cpp"));
    }
}

void UEVWordEntryWidgetDetailed::NativeConstruct()
{
    Super::NativeConstruct();

    SetEditableFieldsReadOnly(true);

    if (Button_SaveChanges)
    {
        Button_SaveChanges->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UEVWordEntryWidgetDetailed::ShowWordEntry(const FEVVocabularyRecord& Entry)
{
    OriginalRecord = BuildRecordForDetailedDisplay(Entry);
    WorkingRecord = OriginalRecord;

    PopulateWordLevelFields();
    PopulateMeaningList();
}

void UEVWordEntryWidgetDetailed::SetButtonsDisabled(const bool bIsViewButtonDisabled, const bool bIsEditButtonDisabled,
                                                    const bool bIsDeleteButtonDisabled,
                                                    const bool bIsSaveChangesButtonHidden)
{
    if (Button_ViewWord)
    {
        Button_ViewWord->SetIsEnabled(!bIsViewButtonDisabled);
    }

    if (Button_EditWordEntry)
    {
        Button_EditWordEntry->SetIsEnabled(!bIsEditButtonDisabled);
    }

    if (Button_DeleteWordEntry)
    {
        Button_DeleteWordEntry->SetIsEnabled(!bIsDeleteButtonDisabled);
    }

    if (Button_SaveChanges)
    {
        Button_SaveChanges->SetVisibility(bIsSaveChangesButtonHidden ? ESlateVisibility::Hidden
                                                                     : ESlateVisibility::Visible);
    }
}

void UEVWordEntryWidgetDetailed::SetEditableFieldsReadOnly(const bool bSetReadOnly)
{
    bMeaningFieldsReadOnly = bSetReadOnly;
    ApplyMeaningEditableState();
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
    FEVVocabularyRecord SubmittedRecord = WorkingRecord;
    NormalizeEditableCollections(SubmittedRecord);

    OnWordEntryChangesSubmitted.Broadcast(SubmittedRecord);
}

void UEVWordEntryWidgetDetailed::HandleDeletePressed()
{
    OnDeleteRequested.Broadcast();
}

void UEVWordEntryWidgetDetailed::HandleMeaningChanged(UEVVocabularyEntryMeaningWidget* MeaningWidget,
                                                      const FEVVocabularyMeaning& UpdatedMeaning)
{
    if (!MeaningWidget)
    {
        return;
    }

    const int32 MeaningIndex = MeaningWidget->GetMeaningIndex();

    if (!WorkingRecord.Meanings.IsValidIndex(MeaningIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid meaning index %d in EVWordEntryWidgetDetailed."), MeaningIndex);
        return;
    }

    WorkingRecord.Meanings[MeaningIndex] = UpdatedMeaning;
}

void UEVWordEntryWidgetDetailed::HandleMeaningEntryWidgetGenerated(UUserWidget& Widget)
{
    UEVVocabularyEntryMeaningWidget* MeaningWidget = Cast<UEVVocabularyEntryMeaningWidget>(&Widget);

    if (!MeaningWidget)
    {
        return;
    }

    MeaningWidget->OnMeaningChanged.AddUniqueDynamic(this, &ThisClass::HandleMeaningChanged);

    MeaningWidget->SetEditable(!bMeaningFieldsReadOnly);
}

void UEVWordEntryWidgetDetailed::PopulateWordLevelFields()
{
    if (TextBlock_Word_Value)
    {
        TextBlock_Word_Value->SetText(
            FText::FromString(EVVocabularyUiStyle::BuildWrappedWordForDisplay(WorkingRecord.Word)));
        TextBlock_Word_Value->SetToolTipText(FText::FromString(WorkingRecord.Word));
        TextBlock_Word_Value->SetColorAndOpacity(EVVocabularyUiStyle::GetWordValueTextFontColor());
    }

    const FEVVocabularyPronunciation* Pronunciation = ResolvePrimaryPronunciation();

    const bool bHasTranscription = Pronunciation && !Pronunciation->Transcription.IsEmpty();
    const bool bHasAudioUrl = Pronunciation && !Pronunciation->AudioUrl.IsEmpty();

    if (TextBlock_Transcription_Value)
    {
        TextBlock_Transcription_Value->SetText(FText::FromString(
            bHasTranscription ? Pronunciation->Transcription : TEXT("No transcription was provided")));
        TextBlock_Transcription_Value->SetColorAndOpacity(
            bHasTranscription ? EVVocabularyUiStyle::GetNormalTranscriptionTextFontColor()
                              : EVVocabularyUiStyle::GetMissingWrodEntryTextFontColor());
    }

    if (TextBlock_Pronunciation_Value)
    {
        TextBlock_Pronunciation_Value->SetText(
            FText::FromString(bHasAudioUrl ? Pronunciation->AudioUrl : TEXT("No pronunciation was provided")));
        TextBlock_Pronunciation_Value->SetColorAndOpacity(
            bHasAudioUrl ? EVVocabularyUiStyle::GetNormalTranscriptionTextFontColor()
                         : EVVocabularyUiStyle::GetMissingWrodEntryTextFontColor());
    }
}

void UEVWordEntryWidgetDetailed::PopulateMeaningList()
{
    if (!ListView_MeaningWidgets)
    {
        return;
    }

    ListView_MeaningWidgets->ClearListItems();

    for (int32 MeaningIndex = 0; MeaningIndex < WorkingRecord.Meanings.Num(); ++MeaningIndex)
    {
        UEVEntryItem* MeaningItem = NewObject<UEVEntryItem>(this);

        if (!MeaningItem)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create meaning item in EVWordEntryWidgetDetailed."));
            continue;
        }

        MeaningItem->PayloadType = EEVEntryItemPayloadType::VocabularyMeaning;
        MeaningItem->VocabularyMeaning = WorkingRecord.Meanings[MeaningIndex];
        MeaningItem->VocabularyMeaningIndex = MeaningIndex;

        ListView_MeaningWidgets->AddItem(MeaningItem);
    }
}

void UEVWordEntryWidgetDetailed::ApplyMeaningEditableState()
{
    if (!ListView_MeaningWidgets)
    {
        return;
    }

    const TArray<UUserWidget*> DisplayedWidgets = ListView_MeaningWidgets->GetDisplayedEntryWidgets();

    for (UUserWidget* DisplayedWidget : DisplayedWidgets)
    {
        if (UEVVocabularyEntryMeaningWidget* MeaningWidget = Cast<UEVVocabularyEntryMeaningWidget>(DisplayedWidget))
        {
            MeaningWidget->SetEditable(!bMeaningFieldsReadOnly);
        }
    }
}

const FEVVocabularyPronunciation* UEVWordEntryWidgetDetailed::ResolvePrimaryPronunciation() const
{
    const FString SelectedLanguageCode = ResolveSelectedVocabularyLanguageCode();

    for (const FEVVocabularyPronunciation& Pronunciation : WorkingRecord.Pronunciations)
    {
        if (Pronunciation.LanguageCode.Equals(SelectedLanguageCode, ESearchCase::IgnoreCase) && Pronunciation.bPrimary)
        {
            return &Pronunciation;
        }
    }

    for (const FEVVocabularyPronunciation& Pronunciation : WorkingRecord.Pronunciations)
    {
        if (Pronunciation.LanguageCode.Equals(SelectedLanguageCode, ESearchCase::IgnoreCase))
        {
            return &Pronunciation;
        }
    }

    return nullptr;
}

FString UEVWordEntryWidgetDetailed::ResolveSelectedVocabularyLanguageCode() const
{
    return TEXT("en");
}

FEVVocabularyRecord
UEVWordEntryWidgetDetailed::BuildRecordForDetailedDisplay(const FEVVocabularyRecord& SourceRecord) const
{
    FEVVocabularyRecord Result = SourceRecord;

    if (Result.Meanings.IsEmpty())
    {
        return Result;
    }

    // Detailed View is a representation of stored vocabulary data, not of the
    // currently selected web-translation request targets. Show every stored
    // translation. General translations remain associated with the first
    // meaning until a future explicit translation-display filter is added.
    TArray<FEVVocabularyTranslation> GeneralTranslations = Result.GeneralTranslations;
    EVVocabularyTranslationUtils::NormalizeTranslations(GeneralTranslations);

    for (FEVVocabularyTranslation& Translation : GeneralTranslations)
    {
        if (Translation.TranslationText.IsEmpty())
        {
            continue;
        }

        const bool bAlreadyPresent = Result.Meanings[0].Translations.ContainsByPredicate(
            [&Translation](const FEVVocabularyTranslation& Existing)
            {
                return Existing.TargetLanguage.Equals(Translation.TargetLanguage, ESearchCase::IgnoreCase) &&
                       Existing.TranslationText.Equals(Translation.TranslationText, ESearchCase::IgnoreCase);
            });

        if (!bAlreadyPresent)
        {
            Translation.TargetPartOfSpeech = Result.Meanings[0].PartOfSpeech;
            Translation.DisplayOrder = Result.Meanings[0].Translations.Num();
            Result.Meanings[0].Translations.Add(MoveTemp(Translation));
        }
    }

    Result.GeneralTranslations.Reset();

    for (FEVVocabularyMeaning& Meaning : Result.Meanings)
    {
        EVVocabularyTranslationUtils::NormalizeTranslations(Meaning.Translations);
    }

    return Result;
}

void UEVWordEntryWidgetDetailed::NormalizeEditableCollections(FEVVocabularyRecord& Record) const
{
    for (FEVVocabularyMeaning& Meaning : Record.Meanings)
    {
        NormalizeTranslations(Meaning);
        NormalizeRelations(Meaning, TEXT("synonym"));
        NormalizeRelations(Meaning, TEXT("antonym"));
    }
}

void UEVWordEntryWidgetDetailed::NormalizeTranslations(FEVVocabularyMeaning& Meaning) const
{
    EVVocabularyTranslationUtils::NormalizeTranslations(Meaning.Translations);
}

void UEVWordEntryWidgetDetailed::NormalizeRelations(FEVVocabularyMeaning& Meaning, const FString& RelationType) const
{
    TArray<FEVVocabularyRelation> OtherRelations;
    TArray<FEVVocabularyRelation> NormalizedTargetRelations;

    for (const FEVVocabularyRelation& SourceRelation : Meaning.Relations)
    {
        if (!SourceRelation.RelationType.Equals(RelationType, ESearchCase::IgnoreCase))
        {
            OtherRelations.Add(SourceRelation);
            continue;
        }

        TArray<FString> Values;
        SplitEditableValues(SourceRelation.RelatedWord, Values);

        for (const FString& Value : Values)
        {
            const bool bDuplicate = NormalizedTargetRelations.ContainsByPredicate(
                [&Value](const FEVVocabularyRelation& Existing)
                { return Existing.RelatedWord.Equals(Value, ESearchCase::IgnoreCase); });

            if (bDuplicate)
            {
                continue;
            }

            FEVVocabularyRelation NewRelation = SourceRelation;
            NewRelation.RelatedWord = Value;
            NewRelation.NormalizedRelatedWord = FEVWordInputValidator::NormalizeWordInput(Value);
            NewRelation.RelationType = RelationType;
            NewRelation.DisplayOrder = NormalizedTargetRelations.Num();
            NormalizedTargetRelations.Add(MoveTemp(NewRelation));
        }
    }

    OtherRelations.Append(NormalizedTargetRelations);
    Meaning.Relations = MoveTemp(OtherRelations);
}

void UEVWordEntryWidgetDetailed::SplitEditableValues(const FString& Source, TArray<FString>& OutValues)
{
    OutValues.Reset();

    FString NormalizedSeparators = Source;
    NormalizedSeparators.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
    NormalizedSeparators.ReplaceInline(TEXT("\r"), TEXT("\n"));
    NormalizedSeparators.ReplaceInline(TEXT("\n"), TEXT(","));

    TArray<FString> SplitValues;
    NormalizedSeparators.ParseIntoArray(SplitValues, TEXT(","), true);

    for (FString& Value : SplitValues)
    {
        Value = Value.TrimStartAndEnd();

        if (Value.IsEmpty())
        {
            continue;
        }

        OutValues.AddUnique(Value);
    }
}
