// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EVVocabularyTypes.h"
#include "EVVocabularyValueItemWidgetBase.h"
#include "EVVocabularyItemSynonyms.generated.h"

/**
 * Displays and edits one stored vocabulary synonym.
 *
 * This widget represents one FEVVocabularyRelation whose RelationType
 * is Synonym. Missing-synonym text is handled by the parent Meaning widget.
 */
UCLASS()
class ENHANCEVOCABULARYUI_API UEVVocabularyItemSynonyms : public UEVVocabularyValueItemWidgetBase
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Vocabulary Synonym Item")
    void SetRelation(const FEVVocabularyRelation& InRelation);

    UFUNCTION(BlueprintPure, Category = "Vocabulary Synonym Item")
    const FEVVocabularyRelation& GetRelation() const;

protected:
    virtual void NativeOnInitialized() override;

private:
    UFUNCTION()
    void HandleRelationValueChanged(UEVVocabularyValueItemWidgetBase* ItemWidget, const FString& NewValue);

    UFUNCTION()
    void HandleRelationValueCommitted(UEVVocabularyValueItemWidgetBase* ItemWidget, const FString& CommittedValue,
                                      ETextCommit::Type CommitMethod);

private:
    UPROPERTY(Transient)
    FEVVocabularyRelation Relation;
};