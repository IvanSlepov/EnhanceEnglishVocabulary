// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "EVVocabularyTypes.h"
#include "EVSearchResultsMeaningWidget.generated.h"

class UTextBlock;

/**
 * Read-only ListView entry that displays one vocabulary meaning
 * inside the Add Word search-results panel.
 *
 * One widget represents one FEVVocabularyMeaning:
 *
 * - Part of speech
 * - Numbered definitions
 * - Usage examples mapped to their definition numbers
 * - Translations for the currently selected target language
 * - Synonyms
 * - Antonyms
 *
 * Translations, synonyms and antonyms are rendered as comma-separated text.
 */
UCLASS()
class ENHANCEVOCABULARYUI_API UEVSearchResultsMeaningWidget : public UUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    /**
     * Returns the meaning currently represented by this ListView entry.
     */
    const FEVVocabularyMeaning& GetCurrentMeaning() const;

protected:
    virtual void NativeOnInitialized() override;

    /**
     * Called by ListView whenever this widget receives its item object.
     */
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

private:
    /**
     * Populates every read-only field from CurrentMeaning.
     */
    void PopulateMeaning();

    /**
     * Builds the numbered definitions text.
     *
     * Output:
     *
     * 1. First definition
     * 2. Second definition
     */
    FString BuildDefinitionsText() const;

    /**
     * Builds usage text using definition numbers.
     *
     * When no usage exists in the entire meaning:
     *
     * No usage was provided
     *
     * When at least one usage exists, only existing usage values
     * are rendered, using the number of their associated definition:
     *
     * 2. Usage associated with definition 2
     */
    FString BuildUsageText(bool& bOutHasAnyUsage) const;

    /**
     * Builds comma-separated translations for the active target language.
     *
     * The current implementation resolves Ukrainian.
     * The function body will later read the selected target language
     * from AppSettings without changing the population flow.
     */
    FString BuildTranslationsText(bool& bOutHasTranslations) const;

    /**
     * Builds comma-separated relations matching RelationType.
     */
    FString BuildRelationsText(const FString& RelationType, bool& bOutHasRelations) const;

    /**
     * Central translation-language resolution point.
     *
     * Returns "uk" for the current Add Word implementation.
     * This function will later read AppSettings.
     */

    /**
     * Applies normal or missing-data color to a value TextBlock.
     */
    void ApplyValueColor(UTextBlock* TextBlock, bool bHasValue) const;

private:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_PartOfSpeech_Value = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_Definition_Value = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_Usage_Value = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_Translations_Values = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_Synonyms_Value = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_Antonyms_Value = nullptr;

    UPROPERTY(Transient)
    FEVVocabularyMeaning CurrentMeaning;
};