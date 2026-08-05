// Fill out your copyright notice in the Description page of Project Settings.

#include "EVSearchResultsMeaningWidget.h"

#include "Components/TextBlock.h"
#include "EVEntryItem.h"
#include "EVVocabularyUiStyle.h"

void UEVSearchResultsMeaningWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (!TextBlock_PartOfSpeech_Value)
    {
        UE_LOG(LogTemp, Error, TEXT("TextBlock_PartOfSpeech_Value is nullptr in EVSearchResultsMeaningWidget."));

        return;
    }

    if (!TextBlock_Definition_Value)
    {
        UE_LOG(LogTemp, Error, TEXT("TextBlock_Definition_Value is nullptr in EVSearchResultsMeaningWidget."));

        return;
    }

    if (!TextBlock_Usage_Value)
    {
        UE_LOG(LogTemp, Error, TEXT("TextBlock_Usage_Value is nullptr in EVSearchResultsMeaningWidget."));

        return;
    }

    if (!TextBlock_Translations_Values)
    {
        UE_LOG(LogTemp, Error, TEXT("TextBlock_Translations_Values is nullptr in EVSearchResultsMeaningWidget."));

        return;
    }

    if (!TextBlock_Synonyms_Value)
    {
        UE_LOG(LogTemp, Error, TEXT("TextBlock_Synonyms_Value is nullptr in EVSearchResultsMeaningWidget."));

        return;
    }

    if (!TextBlock_Antonyms_Value)
    {
        UE_LOG(LogTemp, Error, TEXT("TextBlock_Antonyms_Value is nullptr in EVSearchResultsMeaningWidget."));
    }
}

void UEVSearchResultsMeaningWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (ListItemObject)
    {
        const UEVEntryItem* EntryItem = Cast<UEVEntryItem>(ListItemObject);

        if (!EntryItem)
        {
            UE_LOG(LogTemp, Error,
                   TEXT("List item is not a UEVEntryItem in "
                        "EVSearchResultsMeaningWidget."));

            return;
        }

        if (EntryItem->PayloadType != EEVEntryItemPayloadType::VocabularyMeaning)
        {
            UE_LOG(LogTemp, Error,
                   TEXT("UEVEntryItem does not contain a VocabularyMeaning "
                        "payload in EVSearchResultsMeaningWidget."));

            return;
        }

        CurrentMeaning = EntryItem->VocabularyMeaning;

        PopulateMeaning();
    }
}

const FEVVocabularyMeaning& UEVSearchResultsMeaningWidget::GetCurrentMeaning() const
{
    return CurrentMeaning;
}

void UEVSearchResultsMeaningWidget::PopulateMeaning()
{
    if (!TextBlock_PartOfSpeech_Value || !TextBlock_Definition_Value || !TextBlock_Usage_Value ||
        !TextBlock_Translations_Values || !TextBlock_Synonyms_Value || !TextBlock_Antonyms_Value)
    {
        UE_LOG(LogTemp, Error,
               TEXT("Cannot populate EVSearchResultsMeaningWidget because "
                    "one or more bound widgets are nullptr."));

        return;
    }

    TextBlock_PartOfSpeech_Value->SetText(FText::FromString(CurrentMeaning.PartOfSpeech));

    TextBlock_Definition_Value->SetText(FText::FromString(BuildDefinitionsText()));

    bool bHasAnyUsage = false;

    TextBlock_Usage_Value->SetText(FText::FromString(BuildUsageText(bHasAnyUsage)));

    ApplyValueColor(TextBlock_Usage_Value, bHasAnyUsage);

    bool bHasTranslations = false;

    TextBlock_Translations_Values->SetText(FText::FromString(BuildTranslationsText(bHasTranslations)));

    ApplyValueColor(TextBlock_Translations_Values, bHasTranslations);

    bool bHasSynonyms = false;

    TextBlock_Synonyms_Value->SetText(FText::FromString(BuildRelationsText(TEXT("synonym"), bHasSynonyms)));

    ApplyValueColor(TextBlock_Synonyms_Value, bHasSynonyms);

    bool bHasAntonyms = false;

    TextBlock_Antonyms_Value->SetText(FText::FromString(BuildRelationsText(TEXT("antonym"), bHasAntonyms)));

    ApplyValueColor(TextBlock_Antonyms_Value, bHasAntonyms);
}

FString UEVSearchResultsMeaningWidget::BuildDefinitionsText() const
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

FString UEVSearchResultsMeaningWidget::BuildUsageText(bool& bOutHasAnyUsage) const
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

FString UEVSearchResultsMeaningWidget::BuildTranslationsText(bool& bOutHasTranslations) const
{
    bOutHasTranslations = false;

    const FString SelectedLanguageCode = ResolveSelectedTranslationLanguageCode();

    TArray<FString> TranslationValues;

    for (const FEVVocabularyTranslation& Translation : CurrentMeaning.Translations)
    {
        if (!Translation.TargetLanguage.Equals(SelectedLanguageCode, ESearchCase::IgnoreCase))
        {
            continue;
        }

        if (Translation.TranslationText.IsEmpty())
        {
            continue;
        }

        TranslationValues.AddUnique(Translation.TranslationText);
    }

    bOutHasTranslations = !TranslationValues.IsEmpty();

    if (!bOutHasTranslations)
    {
        return TEXT("No translation was provided");
    }

    return FString::Join(TranslationValues, TEXT(", "));
}

FString UEVSearchResultsMeaningWidget::BuildRelationsText(const FString& RelationType, bool& bOutHasRelations) const
{
    bOutHasRelations = false;

    TArray<FString> RelationValues;

    for (const FEVVocabularyRelation& Relation : CurrentMeaning.Relations)
    {
        if (!Relation.RelationType.Equals(RelationType, ESearchCase::IgnoreCase))
        {
            continue;
        }

        if (Relation.RelatedWord.IsEmpty())
        {
            continue;
        }

        RelationValues.AddUnique(Relation.RelatedWord);
    }

    bOutHasRelations = !RelationValues.IsEmpty();

    if (!bOutHasRelations)
    {
        return RelationType.Equals(TEXT("synonym"), ESearchCase::IgnoreCase) ? TEXT("No synonym was provided")
                                                                             : TEXT("No antonym was provided");
    }

    return FString::Join(RelationValues, TEXT(", "));
}

FString UEVSearchResultsMeaningWidget::ResolveSelectedTranslationLanguageCode() const
{
    return TEXT("uk");
}

void UEVSearchResultsMeaningWidget::ApplyValueColor(UTextBlock* TextBlock, const bool bHasValue) const
{
    if (!TextBlock)
    {
        return;
    }

    TextBlock->SetColorAndOpacity(bHasValue ? EVVocabularyUiStyle::GetNormalWrodEntryTextFontColor()
                                            : EVVocabularyUiStyle::GetMissingWrodEntryTextFontColor());
}