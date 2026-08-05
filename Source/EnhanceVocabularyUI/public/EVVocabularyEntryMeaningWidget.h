// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "EVVocabularyTypes.h"
#include "EVVocabularyEntryMeaningWidget.generated.h"

class UMultiLineEditableTextBox;
class UTextBlock;
class UWrapBox;

class UEVVocabularyItemTranslations;
class UEVVocabularyItemSynonyms;
class UEVVocabularyValueAntonyms;
class UEVVocabularyValueItemWidgetBase;
class UEVVocabularyEntryMeaningWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVocabularyMeaningChanged, UEVVocabularyEntryMeaningWidget*,
                                             MeaningWidget, const FEVVocabularyMeaning&, UpdatedMeaning);

/**
 * Reusable widget representing one vocabulary meaning.
 *
 * Used by:
 * - Review Words in read-only mode;
 * - Detailed View in read-only or editable mode.
 *
 * One widget represents:
 * - one part of speech;
 * - all definitions associated with that part of speech;
 * - all usage examples associated with those definitions;
 * - all meaning-specific translations;
 * - all meaning-specific synonyms;
 * - all meaning-specific antonyms.
 */
UCLASS()
class ENHANCEVOCABULARYUI_API UEVVocabularyEntryMeaningWidget : public UUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    /**
     * Assigns one structured meaning directly.
     *
     * This is also usable when the widget is created outside a ListView.
     */
    UFUNCTION(BlueprintCallable, Category = "Vocabulary Meaning")
    void SetMeaning(const FEVVocabularyMeaning& InMeaning);

    /**
     * Returns the current structured meaning, including unsaved edits.
     */
    UFUNCTION(BlueprintPure, Category = "Vocabulary Meaning")
    const FEVVocabularyMeaning& GetMeaning() const;

    /**
     * Controls whether meaning data can be edited.
     *
     * Read-only:
     * - part of speech, definitions and usages are read-only;
     * - translation/synonym/antonym items are read-only;
     * - child delete buttons are collapsed.
     *
     * Editable:
     * - all meaning fields become editable;
     * - child delete buttons become visible.
     */
    UFUNCTION(BlueprintCallable, Category = "Vocabulary Meaning")
    void SetEditable(bool bInEditable);

    UFUNCTION(BlueprintPure, Category = "Vocabulary Meaning")
    bool IsEditable() const;

    /** Index inside the owning vocabulary record. */
    UFUNCTION(BlueprintPure, Category = "Vocabulary Meaning")
    int32 GetMeaningIndex() const;

    /**
     * Fired whenever this meaning is edited or one of its child values
     * is deleted.
     */
    UPROPERTY(BlueprintAssignable, Category = "Vocabulary Meaning|Events")
    FOnVocabularyMeaningChanged OnMeaningChanged;

protected:
    virtual void NativeOnInitialized() override;

    /**
     * Called automatically when used as a ListView entry.
     */
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

private:
    void PopulateMeaning();

    void PopulateMainTextFields();
    void PopulateTranslations();
    void PopulateSynonyms();
    void PopulateAntonyms();

    void AddTranslationInputSlot();
    void AddSynonymInputSlot();
    void AddAntonymInputSlot();

    int32 GetNextTranslationDisplayOrder() const;
    int32 GetNextRelationDisplayOrder(const FString& RelationType) const;

    void ClearValueCollections();

    /**
     * Builds numbered definitions:
     *
     * 1. Definition A
     * 2. Definition B
     */
    FString BuildDefinitionsText() const;

    /**
     * Builds only existing usage examples using the matching definition
     * numbers.
     *
     * When no usage exists anywhere:
     *
     * No usage was provided
     */
    FString BuildUsageText(bool& bOutHasAnyUsage) const;

    /**
     * Converts an edited numbered definitions field back into the current
     * definition collection.
     *
     * Existing usage mappings are retained by definition index.
     */
    void ApplyDefinitionsText(const FString& InText);

    /**
     * Converts an edited numbered usage field back into usage examples,
     * preserving the displayed definition numbers.
     */
    void ApplyUsageText(const FString& InText);

    /**
     * Creates a red fallback text inside a WrapBox.
     *
     * No value-item widget is created for missing data.
     */
    void AddMissingValueText(UWrapBox* TargetWrapBox, const FText& MissingText);

    void ApplyEditableState();

    void BroadcastMeaningChanged();

    FString NormalizeRelationType(const FString& RelationType) const;

private:
    UFUNCTION()
    void HandlePartOfSpeechTextChanged(const FText& NewText);

    UFUNCTION()
    void HandleDefinitionsTextChanged(const FText& NewText);

    UFUNCTION()
    void HandleUsageTextChanged(const FText& NewText);

    UFUNCTION()
    void HandleTranslationDeleteRequested(UEVVocabularyValueItemWidgetBase* ItemWidget, const FString& Value);

    UFUNCTION()
    void HandleSynonymDeleteRequested(UEVVocabularyValueItemWidgetBase* ItemWidget, const FString& Value);

    UFUNCTION()
    void HandleAntonymDeleteRequested(UEVVocabularyValueItemWidgetBase* ItemWidget, const FString& Value);

    UFUNCTION()
    void HandleChildValueChanged(UEVVocabularyValueItemWidgetBase* ItemWidget, const FString& NewValue);

private:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UMultiLineEditableTextBox> MultiLineEditableTextBox_PartOfSpeech_Value = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UMultiLineEditableTextBox> MultiLineEditableTextBox_Definition_Value = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UMultiLineEditableTextBox> MultiLineEditableTextBox_Usage_Value = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWrapBox> WrapBox_TranslationItem = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWrapBox> WrapBox_SynonymItem = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWrapBox> WrapBox_AntonymItem = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vocabulary Meaning|Item Classes",
              meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UEVVocabularyItemTranslations> TranslationItemWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vocabulary Meaning|Item Classes",
              meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UEVVocabularyItemSynonyms> SynonymItemWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vocabulary Meaning|Item Classes",
              meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UEVVocabularyValueAntonyms> AntonymItemWidgetClass;

    UPROPERTY(Transient)
    FEVVocabularyMeaning CurrentMeaning;

    UPROPERTY(Transient)
    bool bEditable = false;

    UPROPERTY(Transient)
    int32 MeaningIndex = INDEX_NONE;

    /**
     * Prevents programmatic SetText calls during population from being
     * interpreted as user edits.
     */
    UPROPERTY(Transient)
    bool bApplyingMeaningData = false;
};