// Fill out your copyright notice in the Description page of Project Settings.

#include "EVVocabularyItemTranslations.h"

void UEVVocabularyItemTranslations::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleTranslationValueChanged);

    OnValueCommitted.AddUniqueDynamic(this, &ThisClass::HandleTranslationValueCommitted);
}

void UEVVocabularyItemTranslations::SetTranslation(const FEVVocabularyTranslation& InTranslation)
{
    Translation = InTranslation;

    SetValue(Translation.TranslationText);
}

const FEVVocabularyTranslation& UEVVocabularyItemTranslations::GetTranslation() const
{
    return Translation;
}

void UEVVocabularyItemTranslations::HandleTranslationValueChanged(UEVVocabularyValueItemWidgetBase* ItemWidget,
                                                                  const FString& NewValue)
{
    if (ItemWidget != this)
    {
        return;
    }

    Translation.TranslationText = NewValue;
}

void UEVVocabularyItemTranslations::HandleTranslationValueCommitted(UEVVocabularyValueItemWidgetBase* ItemWidget,
                                                                    const FString& CommittedValue,
                                                                    ETextCommit::Type CommitMethod)
{
    if (ItemWidget != this)
    {
        return;
    }

    Translation.TranslationText = CommittedValue;
}