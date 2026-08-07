// Fill out your copyright notice in the Description page of Project Settings.

#include "EVVocabularyValueAntonyms.h"

void UEVVocabularyValueAntonyms::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleRelationValueChanged);

    OnValueCommitted.AddUniqueDynamic(this, &ThisClass::HandleRelationValueCommitted);
}

void UEVVocabularyValueAntonyms::SetRelation(const FEVVocabularyRelation& InRelation)
{
    Relation = InRelation;

    SetValue(Relation.RelatedWord);
}

const FEVVocabularyRelation& UEVVocabularyValueAntonyms::GetRelation() const
{
    return Relation;
}

void UEVVocabularyValueAntonyms::HandleRelationValueChanged(UEVVocabularyValueItemWidgetBase* ItemWidget,
                                                            const FString& NewValue)
{
    if (ItemWidget != this)
    {
        return;
    }

    Relation.RelatedWord = NewValue;
}

void UEVVocabularyValueAntonyms::HandleRelationValueCommitted(UEVVocabularyValueItemWidgetBase* ItemWidget,
                                                              const FString& CommittedValue,
                                                              ETextCommit::Type CommitMethod)
{
    if (ItemWidget != this)
    {
        return;
    }

    Relation.RelatedWord = CommittedValue;
}