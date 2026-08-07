// Fill out your copyright notice in the Description page of Project Settings.

#include "EVVocabularyItemSynonyms.h"

void UEVVocabularyItemSynonyms::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleRelationValueChanged);

    OnValueCommitted.AddUniqueDynamic(this, &ThisClass::HandleRelationValueCommitted);
}

void UEVVocabularyItemSynonyms::SetRelation(const FEVVocabularyRelation& InRelation)
{
    Relation = InRelation;

    SetValue(Relation.RelatedWord);
}

const FEVVocabularyRelation& UEVVocabularyItemSynonyms::GetRelation() const
{
    return Relation;
}

void UEVVocabularyItemSynonyms::HandleRelationValueChanged(UEVVocabularyValueItemWidgetBase* ItemWidget,
                                                           const FString& NewValue)
{
    if (ItemWidget != this)
    {
        return;
    }

    Relation.RelatedWord = NewValue;
}

void UEVVocabularyItemSynonyms::HandleRelationValueCommitted(UEVVocabularyValueItemWidgetBase* ItemWidget,
                                                             const FString& CommittedValue,
                                                             ETextCommit::Type CommitMethod)
{
    if (ItemWidget != this)
    {
        return;
    }

    Relation.RelatedWord = CommittedValue;
}