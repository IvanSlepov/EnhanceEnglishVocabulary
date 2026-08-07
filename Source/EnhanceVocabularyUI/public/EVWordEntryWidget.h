// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "EVEntryItem.h"
#include "EVSearchResultsMeaningWidget.h"
#include "EVVocabularyTypes.h"
#include "EVVocabularyInteractionTypes.h"
#include "EVWordEntryWidget.generated.h"

class UEVWordEntryWidget;

/**
 * Fired when the user presses View on a Review Words entry.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWordEntryViewButtonPressed, UEVWordEntryWidget*, WordEntryWidget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWordEntryValueActionRequested, const FEVVocabularyValueActionRequest&,
                                            Request);

/**
 * Read-only vocabulary entry used by Review Words.
 *
 * Displays:
 * - Word
 * - Primary transcription
 * - Primary pronunciation AudioUrl
 * - One UEVSearchResultsMeaningWidget per vocabulary meaning
 *
 * The widget receives one complete FEVVocabularyRecord through UEVEntryItem.
 */
UCLASS()
class ENHANCEVOCABULARYUI_API UEVWordEntryWidget : public UUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UButton> Button_ViewWord = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_Word_Value = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_Transcription_Value = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_Pronunciation_Value = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UListView> ListView_Meanings = nullptr;

    UPROPERTY(BlueprintAssignable)
    FOnWordEntryViewButtonPressed OnWordEntryViewButtonPressed;

    UPROPERTY(BlueprintAssignable)
    FOnWordEntryValueActionRequested OnValueActionRequested;

    /**
     * Returns the complete structured record represented by this widget.
     */
    const FEVVocabularyRecord& GetCurrentVocabularyRecord() const;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

    /**
     * Called automatically by ListView when this entry receives
     * its UEVEntryItem payload.
     */
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

private:
    /**
     * Populates word-level fields and the meanings ListView.
     */
    void PopulateVocabularyRecord();

    /**
     * Populates:
     * - primary transcription;
     * - primary pronunciation AudioUrl.
     */
    void PopulatePronunciationFields();

    /**
     * Creates one UEVEntryItem per FEVVocabularyMeaning.
     */
    void PopulateMeanings();

    /**
     * Resolves the pronunciation used by the current language mode.
     *
     * Selection order:
     * 1. matching language + bPrimary;
     * 2. first pronunciation matching the language;
     * 3. nullptr.
     */
    const FEVVocabularyPronunciation* ResolvePrimaryPronunciation() const;

    /**
     * Current vocabulary language mode.
     *
     * Temporarily returns English until AppSettings language mode is implemented.
     */
    FString ResolveSelectedVocabularyLanguageCode() const;

    /**
     * Current temporary translation target.
     *
     * Hardcoded to Ukrainian until AppSettings translation selection
     * is implemented.
     */
    FString ResolveSelectedTranslationLanguageCode() const;

    /**
     * Filters meaning-specific translations by the current target language.
     * Until translation providers return part-of-speech-aware values, the
     * entry-level Ukrainian translation is displayed only in the first
     * meaning block.
     */
    FEVVocabularyMeaning BuildMeaningForDisplay(const FEVVocabularyMeaning& SourceMeaning, bool bIsFirstMeaning) const;

    UFUNCTION()
    void HandleOnWordEntry_ViewButtonPressed();

    void HandleMeaningEntryWidgetGenerated(UUserWidget& Widget);

    UFUNCTION()
    void HandleTranslationPressed(const FEVVocabularyTranslation& Translation);

    UFUNCTION()
    void HandleRelationPressed(EEVVocabularyValueActionType ActionType, const FEVVocabularyRelation& Relation);

private:
    UPROPERTY(Transient)
    FEVVocabularyRecord CurrentVocabularyRecord;

    bool bAreRequiredWidgetsCreated = false;
};