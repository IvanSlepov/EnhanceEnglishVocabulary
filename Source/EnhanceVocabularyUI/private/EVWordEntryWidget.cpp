// Fill out your copyright notice in the Description page of Project Settings.

#include "EVWordEntryWidget.h"

#include "EVEntryItem.h"
#include "EVVocabularyUiStyle.h"
#include "EVVocabularyEntryMeaningWidget.h"

void UEVWordEntryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    bAreRequiredWidgetsCreated = Button_ViewWord && TextBlock_Word_Value && TextBlock_Transcription_Value &&
                                 TextBlock_Pronunciation_Value && ListView_Meanings;

    if (!bAreRequiredWidgetsCreated)
    {
        UE_LOG(LogTemp, Error, TEXT("UEVWordEntryWidget: one or more required widgets are missing."));
    }

    if (ListView_Meanings)
    {
        ListView_Meanings->OnEntryWidgetGenerated().AddUObject(this, &ThisClass::HandleMeaningEntryWidgetGenerated);
    }

    if (Button_ViewWord)
    {
        Button_ViewWord->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleOnWordEntry_ViewButtonPressed);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_ViewWord is nullptr in EVWordEntryWidget."));
    }
}

void UEVWordEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UEVWordEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (!bAreRequiredWidgetsCreated)
    {
        return;
    }

    const UEVEntryItem* EntryItem = Cast<UEVEntryItem>(ListItemObject);

    if (!EntryItem)
    {
        UE_LOG(LogTemp, Error, TEXT("List item is not UEVEntryItem in EVWordEntryWidget."));
        return;
    }

    if (EntryItem->PayloadType != EEVEntryItemPayloadType::VocabularyRecord)
    {
        UE_LOG(LogTemp, Error, TEXT("UEVEntryItem does not contain VocabularyRecord payload in EVWordEntryWidget."));
        return;
    }

    CurrentVocabularyRecord = EntryItem->VocabularyRecord;
    PopulateVocabularyRecord();
}

const FEVVocabularyRecord& UEVWordEntryWidget::GetCurrentVocabularyRecord() const
{
    return CurrentVocabularyRecord;
}

void UEVWordEntryWidget::PopulateVocabularyRecord()
{
    const FString DisplayWord = EVVocabularyUiStyle::BuildWrappedWordForDisplay(CurrentVocabularyRecord.Word);

    TextBlock_Word_Value->SetText(FText::FromString(DisplayWord));
    TextBlock_Word_Value->SetToolTipText(FText::FromString(CurrentVocabularyRecord.Word));
    TextBlock_Word_Value->SetColorAndOpacity(EVVocabularyUiStyle::GetWordValueTextFontColor());

    PopulatePronunciationFields();
    PopulateMeanings();
}

void UEVWordEntryWidget::PopulatePronunciationFields()
{
    const FEVVocabularyPronunciation* Pronunciation = ResolvePrimaryPronunciation();

    const bool bHasTranscription = Pronunciation && !Pronunciation->Transcription.IsEmpty();

    TextBlock_Transcription_Value->SetText(
        FText::FromString(bHasTranscription ? Pronunciation->Transcription : TEXT("No transcription was provided")));

    TextBlock_Transcription_Value->SetColorAndOpacity(bHasTranscription
                                                          ? EVVocabularyUiStyle::GetNormalTranscriptionTextFontColor()
                                                          : EVVocabularyUiStyle::GetMissingWrodEntryTextFontColor());

    const bool bHasPronunciation = Pronunciation && !Pronunciation->AudioUrl.IsEmpty();

    TextBlock_Pronunciation_Value->SetText(
        FText::FromString(bHasPronunciation ? Pronunciation->AudioUrl : TEXT("No pronunciation was provided")));

    TextBlock_Pronunciation_Value->SetColorAndOpacity(bHasPronunciation
                                                          ? EVVocabularyUiStyle::GetNormalWrodEntryTextFontColor()
                                                          : EVVocabularyUiStyle::GetMissingWrodEntryTextFontColor());
}

void UEVWordEntryWidget::PopulateMeanings()
{
    if (!ListView_Meanings)
    {
        return;
    }

    ListView_Meanings->ClearListItems();

    for (int32 MeaningIndex = 0; MeaningIndex < CurrentVocabularyRecord.Meanings.Num(); ++MeaningIndex)
    {
        const FEVVocabularyMeaning& Meaning = CurrentVocabularyRecord.Meanings[MeaningIndex];

        UEVEntryItem* Item = NewObject<UEVEntryItem>(this);

        if (!Item)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create meaning item in EVWordEntryWidget."));
            continue;
        }

        Item->PayloadType = EEVEntryItemPayloadType::VocabularyMeaning;
        Item->VocabularyMeaning = BuildMeaningForDisplay(Meaning, MeaningIndex == 0);
        Item->VocabularyMeaningIndex = MeaningIndex;
        Item->MeaningWidgetMode = EEVVocabularyMeaningWidgetMode::ReviewReadOnly;
        ListView_Meanings->AddItem(Item);
    }
}

const FEVVocabularyPronunciation* UEVWordEntryWidget::ResolvePrimaryPronunciation() const
{
    const FString LanguageCode = ResolveSelectedVocabularyLanguageCode();

    for (const FEVVocabularyPronunciation& Pronunciation : CurrentVocabularyRecord.Pronunciations)
    {
        if (Pronunciation.LanguageCode.Equals(LanguageCode, ESearchCase::IgnoreCase) && Pronunciation.bPrimary)
        {
            return &Pronunciation;
        }
    }

    for (const FEVVocabularyPronunciation& Pronunciation : CurrentVocabularyRecord.Pronunciations)
    {
        if (Pronunciation.LanguageCode.Equals(LanguageCode, ESearchCase::IgnoreCase))
        {
            return &Pronunciation;
        }
    }

    return nullptr;
}

FString UEVWordEntryWidget::ResolveSelectedVocabularyLanguageCode() const
{
    // Temporary source-language mode. This will be read from AppSettings
    // when vocabulary language modes are introduced.
    return TEXT("en");
}

FString UEVWordEntryWidget::ResolveSelectedTranslationLanguageCode() const
{
    // Temporary translation target. This will be read from AppSettings
    // when translation-language selection is introduced.
    return TEXT("uk");
}

FEVVocabularyMeaning UEVWordEntryWidget::BuildMeaningForDisplay(const FEVVocabularyMeaning& SourceMeaning,
                                                                const bool bIsFirstMeaning) const
{
    FEVVocabularyMeaning Result = SourceMeaning;

    const FString SelectedLanguageCode = ResolveSelectedTranslationLanguageCode();

    Result.Translations.RemoveAll(
        [&SelectedLanguageCode](const FEVVocabularyTranslation& Translation)
        { return !Translation.TargetLanguage.Equals(SelectedLanguageCode, ESearchCase::IgnoreCase); });

    if (!Result.Translations.IsEmpty() || !bIsFirstMeaning)
    {
        return Result;
    }

    for (const FEVVocabularyTranslation& GeneralTranslation : CurrentVocabularyRecord.GeneralTranslations)
    {
        if (!GeneralTranslation.TargetLanguage.Equals(SelectedLanguageCode, ESearchCase::IgnoreCase) ||
            GeneralTranslation.TranslationText.IsEmpty())
        {
            continue;
        }

        const bool bAlreadyAdded = Result.Translations.ContainsByPredicate(
            [&GeneralTranslation](const FEVVocabularyTranslation& ExistingTranslation)
            {
                return ExistingTranslation.TargetLanguage.Equals(GeneralTranslation.TargetLanguage,
                                                                 ESearchCase::IgnoreCase) &&
                       ExistingTranslation.TranslationText.Equals(GeneralTranslation.TranslationText,
                                                                  ESearchCase::IgnoreCase);
            });

        if (!bAlreadyAdded)
        {
            Result.Translations.Add(GeneralTranslation);
        }
    }

    return Result;
}

void UEVWordEntryWidget::HandleOnWordEntry_ViewButtonPressed()
{
    OnWordEntryViewButtonPressed.Broadcast(this);
}

void UEVWordEntryWidget::HandleMeaningEntryWidgetGenerated(UUserWidget& Widget)
{
    if (UEVVocabularyEntryMeaningWidget* MeaningWidget = Cast<UEVVocabularyEntryMeaningWidget>(&Widget))
    {
        MeaningWidget->OnTranslationPressed.AddUniqueDynamic(this, &ThisClass::HandleTranslationPressed);
        MeaningWidget->OnRelationPressed.AddUniqueDynamic(this, &ThisClass::HandleRelationPressed);
    }
}

void UEVWordEntryWidget::HandleTranslationPressed(const FEVVocabularyTranslation& Translation)
{
    FEVVocabularyValueActionRequest Request;
    Request.ActionType = EEVVocabularyValueActionType::Translation;
    Request.SourceWord = CurrentVocabularyRecord.Word;
    Request.Translation = Translation;
    OnValueActionRequested.Broadcast(Request);
}

void UEVWordEntryWidget::HandleRelationPressed(const EEVVocabularyValueActionType ActionType,
                                               const FEVVocabularyRelation& Relation)
{
    FEVVocabularyValueActionRequest Request;
    Request.ActionType = ActionType;
    Request.SourceWord = CurrentVocabularyRecord.Word;
    Request.Relation = Relation;
    OnValueActionRequested.Broadcast(Request);
}
