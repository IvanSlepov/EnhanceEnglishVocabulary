// Fill out your copyright notice in the Description page of Project Settings.

#include "EVVocabularyEntryMeaningWidget.h"
#include "EVVocabularyTranslationUtils.h"

#include "Blueprint/WidgetTree.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "EVEntryItem.h"
#include "EVVocabularyItemSynonyms.h"
#include "EVVocabularyItemTranslations.h"
#include "EVVocabularyUiStyle.h"
#include "EVVocabularyValueAntonyms.h"
#include "EVVocabularyValueItemWidgetBase.h"

namespace
{
void ApplyReadOnlyBackgroundTint(UMultiLineEditableTextBox* TextBox, const FSlateColor& TintColor)
{
    if (!TextBox)
    {
        return;
    }

    TextBox->WidgetStyle.BackgroundImageReadOnly.TintColor = TintColor;
    TextBox->SynchronizeProperties();
}

FString RemoveNumberPrefix(const FString& SourceText)
{
    FString Result = SourceText.TrimStartAndEnd();

    int32 DotIndex = INDEX_NONE;

    if (!Result.FindChar(TEXT('.'), DotIndex))
    {
        return Result;
    }

    const FString Prefix = Result.Left(DotIndex).TrimStartAndEnd();

    if (!Prefix.IsNumeric())
    {
        return Result;
    }

    return Result.Mid(DotIndex + 1).TrimStartAndEnd();
}

bool TryGetNumberedLineIndex(const FString& SourceText, int32& OutZeroBasedIndex, FString& OutValue)
{
    OutZeroBasedIndex = INDEX_NONE;
    OutValue.Empty();

    const FString TrimmedText = SourceText.TrimStartAndEnd();

    int32 DotIndex = INDEX_NONE;

    if (!TrimmedText.FindChar(TEXT('.'), DotIndex))
    {
        return false;
    }

    const FString Prefix = TrimmedText.Left(DotIndex).TrimStartAndEnd();

    if (!Prefix.IsNumeric())
    {
        return false;
    }

    const int32 OneBasedIndex = FCString::Atoi(*Prefix);

    if (OneBasedIndex <= 0)
    {
        return false;
    }

    OutZeroBasedIndex = OneBasedIndex - 1;
    OutValue = TrimmedText.Mid(DotIndex + 1).TrimStartAndEnd();

    return !OutValue.IsEmpty();
}
} // namespace

void UEVVocabularyEntryMeaningWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (!MultiLineEditableTextBox_PartOfSpeech_Value)
    {
        UE_LOG(LogTemp, Error,
               TEXT("MultiLineEditableTextBox_PartOfSpeech_Value is nullptr "
                    "in EVVocabularyEntryMeaningWidget."));
    }
    else
    {
        MultiLineEditableTextBox_PartOfSpeech_Value->OnTextChanged.AddUniqueDynamic(
            this, &ThisClass::HandlePartOfSpeechTextChanged);
    }

    if (!MultiLineEditableTextBox_Definition_Value)
    {
        UE_LOG(LogTemp, Error,
               TEXT("MultiLineEditableTextBox_Definition_Value is nullptr "
                    "in EVVocabularyEntryMeaningWidget."));
    }
    else
    {
        MultiLineEditableTextBox_Definition_Value->OnTextChanged.AddUniqueDynamic(
            this, &ThisClass::HandleDefinitionsTextChanged);
    }

    if (!MultiLineEditableTextBox_Usage_Value)
    {
        UE_LOG(LogTemp, Error,
               TEXT("MultiLineEditableTextBox_Usage_Value is nullptr "
                    "in EVVocabularyEntryMeaningWidget."));
    }
    else
    {
        MultiLineEditableTextBox_Usage_Value->OnTextChanged.AddUniqueDynamic(this, &ThisClass::HandleUsageTextChanged);
    }

    if (!WrapBox_TranslationItem)
    {
        UE_LOG(LogTemp, Error,
               TEXT("WrapBox_TranslationItem is nullptr "
                    "in EVVocabularyEntryMeaningWidget."));
    }

    if (!WrapBox_SynonymItem)
    {
        UE_LOG(LogTemp, Error,
               TEXT("WrapBox_SynonymItem is nullptr "
                    "in EVVocabularyEntryMeaningWidget."));
    }

    if (!WrapBox_AntonymItem)
    {
        UE_LOG(LogTemp, Error,
               TEXT("WrapBox_AntonymItem is nullptr "
                    "in EVVocabularyEntryMeaningWidget."));
    }

    if (MultiLineEditableTextBox_PartOfSpeech_Value)
    {
        OriginalPartOfSpeechStyle = MultiLineEditableTextBox_PartOfSpeech_Value->WidgetStyle;
    }

    if (MultiLineEditableTextBox_Definition_Value)
    {
        OriginalDefinitionStyle = MultiLineEditableTextBox_Definition_Value->WidgetStyle;
    }

    if (MultiLineEditableTextBox_Usage_Value)
    {
        OriginalUsageStyle = MultiLineEditableTextBox_Usage_Value->WidgetStyle;
    }

    ApplyEditableState();
}

void UEVVocabularyEntryMeaningWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    const UEVEntryItem* EntryItem = Cast<UEVEntryItem>(ListItemObject);

    if (!EntryItem)
    {
        UE_LOG(LogTemp, Error,
               TEXT("List item is not a UEVEntryItem in "
                    "EVVocabularyEntryMeaningWidget."));

        return;
    }

    if (EntryItem->PayloadType != EEVEntryItemPayloadType::VocabularyMeaning)
    {
        UE_LOG(LogTemp, Error,
               TEXT("UEVEntryItem does not contain a VocabularyMeaning "
                    "payload in EVVocabularyEntryMeaningWidget."));

        return;
    }

    MeaningIndex = EntryItem->VocabularyMeaningIndex;
    WidgetMode = EntryItem->MeaningWidgetMode;
    bEditable = WidgetMode == EEVVocabularyMeaningWidgetMode::DetailedEditable;
    SetMeaning(EntryItem->VocabularyMeaning);
}

void UEVVocabularyEntryMeaningWidget::SetMeaning(const FEVVocabularyMeaning& InMeaning)
{
    CurrentMeaning = InMeaning;
    EVVocabularyTranslationUtils::NormalizeTranslations(CurrentMeaning.Translations);

    PopulateMeaning();
}

const FEVVocabularyMeaning& UEVVocabularyEntryMeaningWidget::GetMeaning() const
{
    return CurrentMeaning;
}

void UEVVocabularyEntryMeaningWidget::SetEditable(const bool bInEditable)
{
    SetWidgetMode(bInEditable ? EEVVocabularyMeaningWidgetMode::DetailedEditable
                              : EEVVocabularyMeaningWidgetMode::DetailedReadOnly);
}

void UEVVocabularyEntryMeaningWidget::SetWidgetMode(const EEVVocabularyMeaningWidgetMode InMode)
{
    WidgetMode = InMode;
    bEditable = WidgetMode == EEVVocabularyMeaningWidgetMode::DetailedEditable;
    PopulateMeaning();
}

EEVVocabularyMeaningWidgetMode UEVVocabularyEntryMeaningWidget::GetWidgetMode() const
{
    return WidgetMode;
}

bool UEVVocabularyEntryMeaningWidget::IsEditable() const
{
    return bEditable;
}

int32 UEVVocabularyEntryMeaningWidget::GetMeaningIndex() const
{
    return MeaningIndex;
}

void UEVVocabularyEntryMeaningWidget::PopulateMeaning()
{
    bApplyingMeaningData = true;

    PopulateMainTextFields();

    ClearValueCollections();

    PopulateTranslations();
    PopulateSynonyms();
    PopulateAntonyms();

    ApplyEditableState();

    bApplyingMeaningData = false;
}

void UEVVocabularyEntryMeaningWidget::PopulateMainTextFields()
{
    if (MultiLineEditableTextBox_PartOfSpeech_Value)
    {
        MultiLineEditableTextBox_PartOfSpeech_Value->SetText(FText::FromString(CurrentMeaning.PartOfSpeech));
    }

    if (MultiLineEditableTextBox_Definition_Value)
    {
        MultiLineEditableTextBox_Definition_Value->SetText(FText::FromString(BuildDefinitionsText()));
    }

    if (MultiLineEditableTextBox_Usage_Value)
    {
        bool bHasAnyUsage = false;

        MultiLineEditableTextBox_Usage_Value->SetText(FText::FromString(BuildUsageText(bHasAnyUsage)));

        MultiLineEditableTextBox_Usage_Value->SetForegroundColor(
            bHasAnyUsage ? EVVocabularyUiStyle::GetNormalWordTextLinearColor()
                         : EVVocabularyUiStyle::GetMissingWordTextLinearColor());
    }
}

void UEVVocabularyEntryMeaningWidget::PopulateTranslations()
{
    if (!WrapBox_TranslationItem)
    {
        return;
    }

    if (!TranslationItemWidgetClass)
    {
        UE_LOG(LogTemp, Error,
               TEXT("TranslationItemWidgetClass is not assigned in "
                    "EVVocabularyEntryMeaningWidget."));
        return;
    }

    TArray<FEVVocabularyTranslation> DisplayTranslations = CurrentMeaning.Translations;
    EVVocabularyTranslationUtils::NormalizeTranslations(DisplayTranslations);

    for (const FEVVocabularyTranslation& Translation : DisplayTranslations)
    {
        if (Translation.TranslationText.IsEmpty())
        {
            continue;
        }

        UEVVocabularyItemTranslations* TranslationWidget =
            CreateWidget<UEVVocabularyItemTranslations>(this, TranslationItemWidgetClass);

        if (!TranslationWidget)
        {
            UE_LOG(LogTemp, Error,
                   TEXT("Failed to create translation item widget in "
                        "EVVocabularyEntryMeaningWidget."));
            continue;
        }

        TranslationWidget->SetTranslation(Translation);
        TranslationWidget->SetItemMode(WidgetMode == EEVVocabularyMeaningWidgetMode::ReviewReadOnly
                                           ? EEVVocabularyValueItemMode::ReviewReadOnly
                                           : (bEditable ? EEVVocabularyValueItemMode::DetailedEditable
                                                        : EEVVocabularyValueItemMode::DetailedReadOnly));
        TranslationWidget->OnValuePressed.AddUniqueDynamic(this, &ThisClass::HandleTranslationPressed);
        TranslationWidget->OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleChildValueChanged);
        TranslationWidget->OnDeleteRequested.AddUniqueDynamic(this, &ThisClass::HandleTranslationDeleteRequested);
        WrapBox_TranslationItem->AddChildToWrapBox(TranslationWidget);
    }

    if (bEditable)
    {
        AddTranslationInputSlot();
    }
    else if (WrapBox_TranslationItem->GetChildrenCount() == 0)
    {
        AddMissingValueText(WrapBox_TranslationItem, NSLOCTEXT("EVVocabularyEntryMeaningWidget",
                                                               "NoTranslationProvided", "No translation was provided"));
    }
}

void UEVVocabularyEntryMeaningWidget::PopulateSynonyms()
{
    if (!WrapBox_SynonymItem)
    {
        return;
    }

    if (!SynonymItemWidgetClass)
    {
        UE_LOG(LogTemp, Error,
               TEXT("SynonymItemWidgetClass is not assigned in "
                    "EVVocabularyEntryMeaningWidget."));
        return;
    }

    for (const FEVVocabularyRelation& Relation : CurrentMeaning.Relations)
    {
        if (NormalizeRelationType(Relation.RelationType) != TEXT("synonym") || Relation.RelatedWord.IsEmpty())
        {
            continue;
        }

        UEVVocabularyItemSynonyms* SynonymWidget =
            CreateWidget<UEVVocabularyItemSynonyms>(this, SynonymItemWidgetClass);

        if (!SynonymWidget)
        {
            UE_LOG(LogTemp, Error,
                   TEXT("Failed to create synonym item widget in "
                        "EVVocabularyEntryMeaningWidget."));
            continue;
        }

        SynonymWidget->SetRelation(Relation);
        SynonymWidget->SetItemMode(WidgetMode == EEVVocabularyMeaningWidgetMode::ReviewReadOnly
                                       ? EEVVocabularyValueItemMode::ReviewReadOnly
                                       : (bEditable ? EEVVocabularyValueItemMode::DetailedEditable
                                                    : EEVVocabularyValueItemMode::DetailedReadOnly));
        SynonymWidget->OnValuePressed.AddUniqueDynamic(this, &ThisClass::HandleSynonymPressed);
        SynonymWidget->OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleChildValueChanged);
        SynonymWidget->OnDeleteRequested.AddUniqueDynamic(this, &ThisClass::HandleSynonymDeleteRequested);
        WrapBox_SynonymItem->AddChildToWrapBox(SynonymWidget);
    }

    if (bEditable)
    {
        AddSynonymInputSlot();
    }
    else if (WrapBox_SynonymItem->GetChildrenCount() == 0)
    {
        AddMissingValueText(WrapBox_SynonymItem, NSLOCTEXT("EVVocabularyEntryMeaningWidget", "NoSynonymProvided",
                                                           "No synonym was provided"));
    }
}

void UEVVocabularyEntryMeaningWidget::PopulateAntonyms()
{
    if (!WrapBox_AntonymItem)
    {
        return;
    }

    if (!AntonymItemWidgetClass)
    {
        UE_LOG(LogTemp, Error,
               TEXT("AntonymItemWidgetClass is not assigned in "
                    "EVVocabularyEntryMeaningWidget."));
        return;
    }

    for (const FEVVocabularyRelation& Relation : CurrentMeaning.Relations)
    {
        if (NormalizeRelationType(Relation.RelationType) != TEXT("antonym") || Relation.RelatedWord.IsEmpty())
        {
            continue;
        }

        UEVVocabularyValueAntonyms* AntonymWidget =
            CreateWidget<UEVVocabularyValueAntonyms>(this, AntonymItemWidgetClass);

        if (!AntonymWidget)
        {
            UE_LOG(LogTemp, Error,
                   TEXT("Failed to create antonym item widget in "
                        "EVVocabularyEntryMeaningWidget."));
            continue;
        }

        AntonymWidget->SetRelation(Relation);
        AntonymWidget->SetItemMode(WidgetMode == EEVVocabularyMeaningWidgetMode::ReviewReadOnly
                                       ? EEVVocabularyValueItemMode::ReviewReadOnly
                                       : (bEditable ? EEVVocabularyValueItemMode::DetailedEditable
                                                    : EEVVocabularyValueItemMode::DetailedReadOnly));
        AntonymWidget->OnValuePressed.AddUniqueDynamic(this, &ThisClass::HandleAntonymPressed);
        AntonymWidget->OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleChildValueChanged);
        AntonymWidget->OnDeleteRequested.AddUniqueDynamic(this, &ThisClass::HandleAntonymDeleteRequested);
        WrapBox_AntonymItem->AddChildToWrapBox(AntonymWidget);
    }

    if (bEditable)
    {
        AddAntonymInputSlot();
    }
    else if (WrapBox_AntonymItem->GetChildrenCount() == 0)
    {
        AddMissingValueText(WrapBox_AntonymItem, NSLOCTEXT("EVVocabularyEntryMeaningWidget", "NoAntonymProvided",
                                                           "No antonym was provided"));
    }
}

void UEVVocabularyEntryMeaningWidget::AddTranslationInputSlot()
{
    UEVVocabularyItemTranslations* TranslationWidget =
        CreateWidget<UEVVocabularyItemTranslations>(this, TranslationItemWidgetClass);

    if (!TranslationWidget)
    {
        return;
    }

    FEVVocabularyTranslation PendingTranslation;
    PendingTranslation.TargetLanguage = TEXT("uk");
    PendingTranslation.TargetPartOfSpeech = CurrentMeaning.PartOfSpeech;
    PendingTranslation.DisplayOrder = GetNextTranslationDisplayOrder();

    TranslationWidget->SetTranslation(PendingTranslation);
    TranslationWidget->SetPendingAddItem(true);
    TranslationWidget->SetHintText(NSLOCTEXT("EVVocabularyEntryMeaningWidget", "AddTranslationHint",
                                             "Enter a new translation. Use commas or new lines for several values."));
    TranslationWidget->SetEditable(true);
    TranslationWidget->OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleChildValueChanged);
    WrapBox_TranslationItem->AddChildToWrapBox(TranslationWidget);
}

void UEVVocabularyEntryMeaningWidget::AddSynonymInputSlot()
{
    UEVVocabularyItemSynonyms* SynonymWidget = CreateWidget<UEVVocabularyItemSynonyms>(this, SynonymItemWidgetClass);

    if (!SynonymWidget)
    {
        return;
    }

    FEVVocabularyRelation PendingRelation;
    PendingRelation.RelationType = TEXT("synonym");
    PendingRelation.DisplayOrder = GetNextRelationDisplayOrder(TEXT("synonym"));

    SynonymWidget->SetRelation(PendingRelation);
    SynonymWidget->SetPendingAddItem(true);
    SynonymWidget->SetHintText(NSLOCTEXT("EVVocabularyEntryMeaningWidget", "AddSynonymHint",
                                         "Enter a new synonym. Use commas or new lines for several values."));
    SynonymWidget->SetEditable(true);
    SynonymWidget->OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleChildValueChanged);
    WrapBox_SynonymItem->AddChildToWrapBox(SynonymWidget);
}

void UEVVocabularyEntryMeaningWidget::AddAntonymInputSlot()
{
    UEVVocabularyValueAntonyms* AntonymWidget = CreateWidget<UEVVocabularyValueAntonyms>(this, AntonymItemWidgetClass);

    if (!AntonymWidget)
    {
        return;
    }

    FEVVocabularyRelation PendingRelation;
    PendingRelation.RelationType = TEXT("antonym");
    PendingRelation.DisplayOrder = GetNextRelationDisplayOrder(TEXT("antonym"));

    AntonymWidget->SetRelation(PendingRelation);
    AntonymWidget->SetPendingAddItem(true);
    AntonymWidget->SetHintText(NSLOCTEXT("EVVocabularyEntryMeaningWidget", "AddAntonymHint",
                                         "Enter a new antonym. Use commas or new lines for several values."));
    AntonymWidget->SetEditable(true);
    AntonymWidget->OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleChildValueChanged);
    WrapBox_AntonymItem->AddChildToWrapBox(AntonymWidget);
}

int32 UEVVocabularyEntryMeaningWidget::GetNextTranslationDisplayOrder() const
{
    int32 NextDisplayOrder = 0;

    for (const FEVVocabularyTranslation& Translation : CurrentMeaning.Translations)
    {
        NextDisplayOrder = FMath::Max(NextDisplayOrder, Translation.DisplayOrder + 1);
    }

    return NextDisplayOrder;
}

int32 UEVVocabularyEntryMeaningWidget::GetNextRelationDisplayOrder(const FString& RelationType) const
{
    int32 NextDisplayOrder = 0;
    const FString NormalizedRequestedType = NormalizeRelationType(RelationType);

    for (const FEVVocabularyRelation& Relation : CurrentMeaning.Relations)
    {
        if (NormalizeRelationType(Relation.RelationType) == NormalizedRequestedType)
        {
            NextDisplayOrder = FMath::Max(NextDisplayOrder, Relation.DisplayOrder + 1);
        }
    }

    return NextDisplayOrder;
}

void UEVVocabularyEntryMeaningWidget::ClearValueCollections()
{
    if (WrapBox_TranslationItem)
    {
        WrapBox_TranslationItem->ClearChildren();
    }

    if (WrapBox_SynonymItem)
    {
        WrapBox_SynonymItem->ClearChildren();
    }

    if (WrapBox_AntonymItem)
    {
        WrapBox_AntonymItem->ClearChildren();
    }
}

FString UEVVocabularyEntryMeaningWidget::BuildDefinitionsText() const
{
    FString Result;

    for (int32 DefinitionIndex = 0; DefinitionIndex < CurrentMeaning.Definitions.Num(); ++DefinitionIndex)
    {
        const FEVVocabularyDefinition& Definition = CurrentMeaning.Definitions[DefinitionIndex];

        if (Definition.DefinitionText.IsEmpty())
        {
            continue;
        }

        if (!Result.IsEmpty())
        {
            Result.Append(TEXT("\n\n"));
        }

        Result.Append(FString::Printf(TEXT("%d. %s"), DefinitionIndex + 1, *Definition.DefinitionText));
    }

    return Result;
}

FString UEVVocabularyEntryMeaningWidget::BuildUsageText(bool& bOutHasAnyUsage) const
{
    bOutHasAnyUsage = false;

    FString Result;

    for (int32 DefinitionIndex = 0; DefinitionIndex < CurrentMeaning.Definitions.Num(); ++DefinitionIndex)
    {
        const FEVVocabularyDefinition& Definition = CurrentMeaning.Definitions[DefinitionIndex];

        if (Definition.UsageExample.IsEmpty())
        {
            continue;
        }

        bOutHasAnyUsage = true;

        if (!Result.IsEmpty())
        {
            Result.Append(TEXT("\n\n"));
        }

        Result.Append(FString::Printf(TEXT("%d. %s"), DefinitionIndex + 1, *Definition.UsageExample));
    }

    if (!bOutHasAnyUsage)
    {
        return TEXT("No usage was provided");
    }

    return Result;
}

void UEVVocabularyEntryMeaningWidget::ApplyDefinitionsText(const FString& InText)
{
    TArray<FString> Lines;

    InText.ParseIntoArrayLines(Lines, false);

    TArray<FEVVocabularyDefinition> UpdatedDefinitions;

    for (const FString& Line : Lines)
    {
        const FString DefinitionText = RemoveNumberPrefix(Line);

        if (DefinitionText.IsEmpty())
        {
            continue;
        }

        FEVVocabularyDefinition UpdatedDefinition;

        const int32 NewDefinitionIndex = UpdatedDefinitions.Num();

        if (CurrentMeaning.Definitions.IsValidIndex(NewDefinitionIndex))
        {
            UpdatedDefinition = CurrentMeaning.Definitions[NewDefinitionIndex];
        }

        UpdatedDefinition.DefinitionText = DefinitionText;

        UpdatedDefinition.DisplayOrder = NewDefinitionIndex;

        UpdatedDefinitions.Add(UpdatedDefinition);
    }

    CurrentMeaning.Definitions = MoveTemp(UpdatedDefinitions);
}

void UEVVocabularyEntryMeaningWidget::ApplyUsageText(const FString& InText)
{
    const FString TrimmedText = InText.TrimStartAndEnd();

    for (FEVVocabularyDefinition& Definition : CurrentMeaning.Definitions)
    {
        Definition.UsageExample.Empty();
    }

    if (TrimmedText.IsEmpty() || TrimmedText.Equals(TEXT("No usage was provided"), ESearchCase::IgnoreCase))
    {
        return;
    }

    TArray<FString> Lines;

    InText.ParseIntoArrayLines(Lines, false);

    int32 SequentialDefinitionIndex = 0;

    for (const FString& Line : Lines)
    {
        const FString TrimmedLine = Line.TrimStartAndEnd();

        if (TrimmedLine.IsEmpty())
        {
            continue;
        }

        int32 DefinitionIndex = INDEX_NONE;
        FString UsageValue;

        if (!TryGetNumberedLineIndex(TrimmedLine, DefinitionIndex, UsageValue))
        {
            DefinitionIndex = SequentialDefinitionIndex;

            UsageValue = TrimmedLine;
        }

        ++SequentialDefinitionIndex;

        if (!CurrentMeaning.Definitions.IsValidIndex(DefinitionIndex))
        {
            continue;
        }

        CurrentMeaning.Definitions[DefinitionIndex].UsageExample = UsageValue;
    }
}

void UEVVocabularyEntryMeaningWidget::AddMissingValueText(UWrapBox* TargetWrapBox, const FText& MissingText)
{
    if (!TargetWrapBox || !WidgetTree)
    {
        return;
    }

    UTextBlock* MissingValueTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());

    if (!MissingValueTextBlock)
    {
        UE_LOG(LogTemp, Error,
               TEXT("Failed to create missing-value TextBlock in "
                    "EVVocabularyEntryMeaningWidget."));

        return;
    }

    MissingValueTextBlock->SetText(MissingText);

    MissingValueTextBlock->SetColorAndOpacity(EVVocabularyUiStyle::GetMissingWrodEntryTextFontColor());

    MissingValueTextBlock->SetAutoWrapText(true);

    TargetWrapBox->AddChildToWrapBox(MissingValueTextBlock);
}

void UEVVocabularyEntryMeaningWidget::ApplyEditableState()
{
    const bool bIsReview = WidgetMode == EEVVocabularyMeaningWidgetMode::ReviewReadOnly;
    const bool bIsDetailedEditable = WidgetMode == EEVVocabularyMeaningWidgetMode::DetailedEditable;

    if (MultiLineEditableTextBox_PartOfSpeech_Value)
    {
        MultiLineEditableTextBox_PartOfSpeech_Value->SetIsReadOnly(!bIsDetailedEditable);
        ApplyReadOnlyBackgroundTint(MultiLineEditableTextBox_PartOfSpeech_Value,
                                    bIsReview ? OriginalPartOfSpeechStyle.BackgroundImageNormal.TintColor
                                              : OriginalPartOfSpeechStyle.BackgroundImageReadOnly.TintColor);
    }

    if (MultiLineEditableTextBox_Definition_Value)
    {
        MultiLineEditableTextBox_Definition_Value->SetIsReadOnly(!bIsDetailedEditable);
        ApplyReadOnlyBackgroundTint(MultiLineEditableTextBox_Definition_Value,
                                    bIsReview ? OriginalDefinitionStyle.BackgroundImageNormal.TintColor
                                              : OriginalDefinitionStyle.BackgroundImageReadOnly.TintColor);
    }

    if (MultiLineEditableTextBox_Usage_Value)
    {
        MultiLineEditableTextBox_Usage_Value->SetIsReadOnly(!bIsDetailedEditable);
        ApplyReadOnlyBackgroundTint(MultiLineEditableTextBox_Usage_Value,
                                    bIsReview ? OriginalUsageStyle.BackgroundImageNormal.TintColor
                                              : OriginalUsageStyle.BackgroundImageReadOnly.TintColor);
    }

    const EEVVocabularyValueItemMode ItemMode =
        bIsReview ? EEVVocabularyValueItemMode::ReviewReadOnly
                  : (bIsDetailedEditable ? EEVVocabularyValueItemMode::DetailedEditable
                                         : EEVVocabularyValueItemMode::DetailedReadOnly);

    const auto ApplyStateToWrapBox = [ItemMode](UWrapBox* WrapBox)
    {
        if (!WrapBox)
        {
            return;
        }

        for (int32 ChildIndex = 0; ChildIndex < WrapBox->GetChildrenCount(); ++ChildIndex)
        {
            if (UEVVocabularyValueItemWidgetBase* ItemWidget =
                    Cast<UEVVocabularyValueItemWidgetBase>(WrapBox->GetChildAt(ChildIndex)))
            {
                ItemWidget->SetItemMode(ItemMode);
            }
        }
    };

    ApplyStateToWrapBox(WrapBox_TranslationItem);
    ApplyStateToWrapBox(WrapBox_SynonymItem);
    ApplyStateToWrapBox(WrapBox_AntonymItem);
}

void UEVVocabularyEntryMeaningWidget::BroadcastMeaningChanged()
{
    if (bApplyingMeaningData)
    {
        return;
    }

    OnMeaningChanged.Broadcast(this, CurrentMeaning);
}

void UEVVocabularyEntryMeaningWidget::HandleTranslationPressed(UEVVocabularyValueItemWidgetBase* ItemWidget,
                                                               const FString& Value)
{
    if (WidgetMode != EEVVocabularyMeaningWidgetMode::ReviewReadOnly)
    {
        return;
    }

    if (const UEVVocabularyItemTranslations* TranslationWidget = Cast<UEVVocabularyItemTranslations>(ItemWidget))
    {
        OnTranslationPressed.Broadcast(TranslationWidget->GetTranslation());
    }
}

void UEVVocabularyEntryMeaningWidget::HandleSynonymPressed(UEVVocabularyValueItemWidgetBase* ItemWidget,
                                                           const FString& Value)
{
    if (WidgetMode != EEVVocabularyMeaningWidgetMode::ReviewReadOnly)
    {
        return;
    }

    if (const UEVVocabularyItemSynonyms* SynonymWidget = Cast<UEVVocabularyItemSynonyms>(ItemWidget))
    {
        OnRelationPressed.Broadcast(EEVVocabularyValueActionType::Synonym, SynonymWidget->GetRelation());
    }
}

void UEVVocabularyEntryMeaningWidget::HandleAntonymPressed(UEVVocabularyValueItemWidgetBase* ItemWidget,
                                                           const FString& Value)
{
    if (WidgetMode != EEVVocabularyMeaningWidgetMode::ReviewReadOnly)
    {
        return;
    }

    if (const UEVVocabularyValueAntonyms* AntonymWidget = Cast<UEVVocabularyValueAntonyms>(ItemWidget))
    {
        OnRelationPressed.Broadcast(EEVVocabularyValueActionType::Antonym, AntonymWidget->GetRelation());
    }
}

FString UEVVocabularyEntryMeaningWidget::NormalizeRelationType(const FString& RelationType) const
{
    FString NormalizedType = RelationType.TrimStartAndEnd();

    NormalizedType.ToLowerInline();

    return NormalizedType;
}

void UEVVocabularyEntryMeaningWidget::HandlePartOfSpeechTextChanged(const FText& NewText)
{
    if (bApplyingMeaningData || !bEditable)
    {
        return;
    }

    CurrentMeaning.PartOfSpeech = NewText.ToString();

    BroadcastMeaningChanged();
}

void UEVVocabularyEntryMeaningWidget::HandleDefinitionsTextChanged(const FText& NewText)
{
    if (bApplyingMeaningData || !bEditable)
    {
        return;
    }

    ApplyDefinitionsText(NewText.ToString());

    BroadcastMeaningChanged();
}

void UEVVocabularyEntryMeaningWidget::HandleUsageTextChanged(const FText& NewText)
{
    if (bApplyingMeaningData || !bEditable)
    {
        return;
    }

    ApplyUsageText(NewText.ToString());

    BroadcastMeaningChanged();
}

void UEVVocabularyEntryMeaningWidget::HandleTranslationDeleteRequested(UEVVocabularyValueItemWidgetBase* ItemWidget,
                                                                       const FString& Value)
{
    if (!bEditable)
    {
        return;
    }

    const UEVVocabularyItemTranslations* TranslationWidget = Cast<UEVVocabularyItemTranslations>(ItemWidget);

    if (!TranslationWidget)
    {
        return;
    }

    const FEVVocabularyTranslation TranslationToDelete = TranslationWidget->GetTranslation();

    const int32 RemovedCount = CurrentMeaning.Translations.RemoveAll(
        [&TranslationToDelete](const FEVVocabularyTranslation& Translation)
        {
            return Translation.DisplayOrder == TranslationToDelete.DisplayOrder &&
                   Translation.TargetLanguage.Equals(TranslationToDelete.TargetLanguage, ESearchCase::IgnoreCase) &&
                   Translation.TranslationText.Equals(TranslationToDelete.TranslationText, ESearchCase::CaseSensitive);
        });

    if (RemovedCount == 0)
    {
        return;
    }

    PopulateMeaning();
    BroadcastMeaningChanged();
}

void UEVVocabularyEntryMeaningWidget::HandleSynonymDeleteRequested(UEVVocabularyValueItemWidgetBase* ItemWidget,
                                                                   const FString& Value)
{
    if (!bEditable)
    {
        return;
    }

    const UEVVocabularyItemSynonyms* SynonymWidget = Cast<UEVVocabularyItemSynonyms>(ItemWidget);

    if (!SynonymWidget)
    {
        return;
    }

    const FEVVocabularyRelation RelationToDelete = SynonymWidget->GetRelation();

    const int32 RemovedCount = CurrentMeaning.Relations.RemoveAll(
        [this, &RelationToDelete](const FEVVocabularyRelation& Relation)
        {
            return NormalizeRelationType(Relation.RelationType) == TEXT("synonym") &&
                   Relation.DisplayOrder == RelationToDelete.DisplayOrder &&
                   Relation.RelatedWord.Equals(RelationToDelete.RelatedWord, ESearchCase::CaseSensitive);
        });

    if (RemovedCount == 0)
    {
        return;
    }

    PopulateMeaning();
    BroadcastMeaningChanged();
}

void UEVVocabularyEntryMeaningWidget::HandleAntonymDeleteRequested(UEVVocabularyValueItemWidgetBase* ItemWidget,
                                                                   const FString& Value)
{
    if (!bEditable)
    {
        return;
    }

    const UEVVocabularyValueAntonyms* AntonymWidget = Cast<UEVVocabularyValueAntonyms>(ItemWidget);

    if (!AntonymWidget)
    {
        return;
    }

    const FEVVocabularyRelation RelationToDelete = AntonymWidget->GetRelation();

    const int32 RemovedCount = CurrentMeaning.Relations.RemoveAll(
        [this, &RelationToDelete](const FEVVocabularyRelation& Relation)
        {
            return NormalizeRelationType(Relation.RelationType) == TEXT("antonym") &&
                   Relation.DisplayOrder == RelationToDelete.DisplayOrder &&
                   Relation.RelatedWord.Equals(RelationToDelete.RelatedWord, ESearchCase::CaseSensitive);
        });

    if (RemovedCount == 0)
    {
        return;
    }

    PopulateMeaning();
    BroadcastMeaningChanged();
}

void UEVVocabularyEntryMeaningWidget::HandleChildValueChanged(UEVVocabularyValueItemWidgetBase* ItemWidget,
                                                              const FString& NewValue)
{
    if (!bEditable || !ItemWidget)
    {
        return;
    }

    if (const UEVVocabularyItemTranslations* TranslationWidget = Cast<UEVVocabularyItemTranslations>(ItemWidget))
    {
        FEVVocabularyTranslation UpdatedTranslation = TranslationWidget->GetTranslation();
        UpdatedTranslation.TranslationText = NewValue;

        for (FEVVocabularyTranslation& Translation : CurrentMeaning.Translations)
        {
            if (Translation.DisplayOrder == UpdatedTranslation.DisplayOrder &&
                Translation.TargetLanguage.Equals(UpdatedTranslation.TargetLanguage, ESearchCase::IgnoreCase))
            {
                Translation = UpdatedTranslation;
                BroadcastMeaningChanged();
                return;
            }
        }

        if (ItemWidget->IsPendingAddItem() && !NewValue.TrimStartAndEnd().IsEmpty())
        {
            CurrentMeaning.Translations.Add(UpdatedTranslation);
            BroadcastMeaningChanged();
        }

        return;
    }

    if (const UEVVocabularyItemSynonyms* SynonymWidget = Cast<UEVVocabularyItemSynonyms>(ItemWidget))
    {
        FEVVocabularyRelation UpdatedRelation = SynonymWidget->GetRelation();
        UpdatedRelation.RelatedWord = NewValue;

        for (FEVVocabularyRelation& Relation : CurrentMeaning.Relations)
        {
            if (NormalizeRelationType(Relation.RelationType) == TEXT("synonym") &&
                Relation.DisplayOrder == UpdatedRelation.DisplayOrder)
            {
                Relation = UpdatedRelation;
                BroadcastMeaningChanged();
                return;
            }
        }

        if (ItemWidget->IsPendingAddItem() && !NewValue.TrimStartAndEnd().IsEmpty())
        {
            CurrentMeaning.Relations.Add(UpdatedRelation);
            BroadcastMeaningChanged();
        }

        return;
    }

    if (const UEVVocabularyValueAntonyms* AntonymWidget = Cast<UEVVocabularyValueAntonyms>(ItemWidget))
    {
        FEVVocabularyRelation UpdatedRelation = AntonymWidget->GetRelation();
        UpdatedRelation.RelatedWord = NewValue;

        for (FEVVocabularyRelation& Relation : CurrentMeaning.Relations)
        {
            if (NormalizeRelationType(Relation.RelationType) == TEXT("antonym") &&
                Relation.DisplayOrder == UpdatedRelation.DisplayOrder)
            {
                Relation = UpdatedRelation;
                BroadcastMeaningChanged();
                return;
            }
        }

        if (ItemWidget->IsPendingAddItem() && !NewValue.TrimStartAndEnd().IsEmpty())
        {
            CurrentMeaning.Relations.Add(UpdatedRelation);
            BroadcastMeaningChanged();
        }
    }
}
