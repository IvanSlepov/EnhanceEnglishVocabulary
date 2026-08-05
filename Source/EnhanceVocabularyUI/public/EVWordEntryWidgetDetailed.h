// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVVocabularyTypes.h"
#include "EVWordEntryDisplayWidgetProvider.h"
#include "EVWordEntryWidgetDetailed.generated.h"

class UButton;
class UListView;
class UTextBlock;
class UUserWidget;
class UEVVocabularyEntryMeaningWidget;

/**
 * Detailed vocabulary-entry view.
 *
 * Word, transcription and pronunciation stay read-only.
 * Meaning data is edited through UEVVocabularyEntryMeaningWidget instances.
 * All edits remain local until Save Changes is confirmed by the controller.
 */
UCLASS()
class ENHANCEVOCABULARYUI_API UEVWordEntryWidgetDetailed : public UUserWidget, public IEVWordEntryDisplayWidgetProvider
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UButton> Button_ViewWord = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UButton> Button_EditWordEntry = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UButton> Button_DeleteWordEntry = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UButton> Button_SaveChanges = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_Word_Value = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_Transcription_Value = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_Pronunciation_Value = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UListView> ListView_MeaningWidgets = nullptr;

    virtual void ShowWordEntry(const FEVVocabularyRecord& Entry) override;

    virtual void SetButtonsDisabled(bool bIsViewButtonDisabled, bool bIsEditButtonDisabled,
                                    bool bIsDeleteButtonDisabled, bool bIsSaveChangesButtonHidden) override;

    virtual void SetEditableFieldsReadOnly(bool bSetReadOnly) override;

    FSimpleMulticastDelegate OnViewRequested;
    FSimpleMulticastDelegate OnEditRequested;
    FOnWordEntryChangesSubmitted OnWordEntryChangesSubmitted;
    FSimpleMulticastDelegate OnDeleteRequested;

    virtual FSimpleMulticastDelegate& GetViewPressedDelegate() override
    {
        return OnViewRequested;
    }

    virtual FSimpleMulticastDelegate& GetEditPressedDelegate() override
    {
        return OnEditRequested;
    }

    virtual FOnWordEntryChangesSubmitted& GetSaveChangesSubmittedDelegate() override
    {
        return OnWordEntryChangesSubmitted;
    }

    virtual FSimpleMulticastDelegate& GetDeletePressedDelegate() override
    {
        return OnDeleteRequested;
    }

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

private:
    UFUNCTION()
    void HandleViewPressed();

    UFUNCTION()
    void HandleEditPressed();

    UFUNCTION()
    void HandleSaveChangesPressed();

    UFUNCTION()
    void HandleDeletePressed();

    UFUNCTION()
    void HandleMeaningChanged(UEVVocabularyEntryMeaningWidget* MeaningWidget,
                              const FEVVocabularyMeaning& UpdatedMeaning);

    void HandleMeaningEntryWidgetGenerated(UUserWidget& Widget);

    void PopulateWordLevelFields();
    void PopulateMeaningList();
    void ApplyMeaningEditableState();

    const FEVVocabularyPronunciation* ResolvePrimaryPronunciation() const;
    FString ResolveSelectedVocabularyLanguageCode() const;
    FString ResolveSelectedTranslationLanguageCode() const;
    FEVVocabularyRecord BuildRecordForDetailedDisplay(const FEVVocabularyRecord& SourceRecord) const;

    void NormalizeEditableCollections(FEVVocabularyRecord& Record) const;
    void NormalizeTranslations(FEVVocabularyMeaning& Meaning) const;
    void NormalizeRelations(FEVVocabularyMeaning& Meaning, const FString& RelationType) const;

    static void SplitEditableValues(const FString& Source, TArray<FString>& OutValues);

private:
    UPROPERTY(Transient)
    FEVVocabularyRecord OriginalRecord;

    UPROPERTY(Transient)
    FEVVocabularyRecord WorkingRecord;

    UPROPERTY(Transient)
    bool bMeaningFieldsReadOnly = true;
};
