// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVVocabularyTypes.h"
#include "EVSearchResultsPanel.generated.h"

class UButton;
class UListView;
class UTextBlock;

/**
 * Add Word search-result panel.
 *
 * Displays:
 * - the primary transcription;
 * - the current pronunciation/audio placeholder value;
 * - one read-only meaning widget per FEVVocabularyMeaning.
 *
 * Save and Discard behavior remains owned by UEVAddWordWidget through
 * the existing delegates.
 */

// Use OnClicked because OnPressed may produce stale input behavior on mobile.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveClicked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDiscardClicked);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVSearchResultsPanel : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     * Populates the complete Add Word search-result panel.
     */
    void SetSearchResult(const FWordSearchResult& InSearchResult);

    /**
     * Clears the cached result and every displayed field/list item.
     */
    void ClearSearchResult();

    /**
     * Returns the complete structured record currently displayed.
     */
    const FEVVocabularyRecord& GetCurrentVocabularyRecord() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_SearchResultsTranscription_Value = nullptr;

    /**
     * Temporary pronunciation/audio placeholder.
     *
     * Until audio playback is implemented, this field displays the selected
     * pronunciation audio URL or the missing-pronunciation text.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_SearchResultsPronunciation_Value = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UListView> ListView_SearchResults = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UButton> Button_Save = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UButton> Button_Discard = nullptr;

    UPROPERTY(BlueprintAssignable, Category = "Search Results Panel Button Events")
    FOnSaveClicked OnSaveClicked;

    UPROPERTY(BlueprintAssignable, Category = "Search Results Panel Button Events")
    FOnDiscardClicked OnDiscardClicked;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

private:
    UFUNCTION()
    void HandleSaveClicked();

    UFUNCTION()
    void HandleDiscardClicked();

    /**
     * Populates the transcription and pronunciation fields.
     */
    void PopulatePronunciationFields();

    /**
     * Creates one UEVEntryItem per meaning and adds it to the ListView.
     */
    void PopulateMeaningList();

    /**
     * Returns the primary pronunciation.
     *
     * Selection order within the selected vocabulary language:
     * 1. pronunciation marked bPrimary;
     * 2. first pronunciation matching the language;
     * 3. nullptr when no matching pronunciation exists.
     */
    const FEVVocabularyPronunciation* ResolvePrimaryPronunciation() const;

    /**
     * Returns the currently selected vocabulary/source language.
     *
     * English is the temporary source-language mode for the current Add Word flow.
     * This function is the single replacement point for the future
     * AppSettings language-mode selection.
     */
    FString ResolveSelectedVocabularyLanguageCode() const;

    /**
     * Returns the currently selected translation target language.
     *
     * Ukrainian is fixed for the current Add Word flow.
     * This function is the single replacement point for the future
     * AppSettings translation-language selection.
     */
    TArray<FString> ResolveSelectedTranslationLanguageCodes() const;

    /**
     * Prepares one meaning for Search Results display.
     *
     * Meaning-specific Ukrainian translations remain attached to the meaning.
     * Entry-level translations are not assigned to a meaning without an
     * explicit source-part-of-speech association.
     */
    FEVVocabularyMeaning BuildMeaningForDisplay(const FEVVocabularyMeaning& SourceMeaning, bool bIsFirstMeaning) const;

private:
    UPROPERTY(Transient)
    FEVVocabularyRecord CurrentVocabularyRecord;
};