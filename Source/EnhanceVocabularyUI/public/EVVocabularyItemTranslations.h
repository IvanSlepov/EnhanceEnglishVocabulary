// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EVVocabularyTypes.h"
#include "EVVocabularyValueItemWidgetBase.h"
#include "EVVocabularyItemTranslations.generated.h"

/**
 * Displays and edits one stored vocabulary translation.
 *
 * This widget represents a real FEVVocabularyTranslation entry only.
 * Missing-translation text is handled by the parent Meaning widget.
 */
UCLASS()
class ENHANCEVOCABULARYUI_API UEVVocabularyItemTranslations : public UEVVocabularyValueItemWidgetBase
{
    GENERATED_BODY()

public:
    /**
     * Assigns the structured translation represented by this widget.
     */
    UFUNCTION(BlueprintCallable, Category = "Vocabulary Translation Item")
    void SetTranslation(const FEVVocabularyTranslation& InTranslation);

    /**
     * Returns the structured translation currently represented.
     */
    UFUNCTION(BlueprintPure, Category = "Vocabulary Translation Item")
    const FEVVocabularyTranslation& GetTranslation() const;

protected:
    /**
     * Synchronizes TranslationText when the base value changes.
     */
    virtual void NativeOnInitialized() override;

private:
    UFUNCTION()
    void HandleTranslationValueChanged(UEVVocabularyValueItemWidgetBase* ItemWidget, const FString& NewValue);

    UFUNCTION()
    void HandleTranslationValueCommitted(UEVVocabularyValueItemWidgetBase* ItemWidget, const FString& CommittedValue,
                                         ETextCommit::Type CommitMethod);

private:
    UPROPERTY(Transient)
    FEVVocabularyTranslation Translation;
};