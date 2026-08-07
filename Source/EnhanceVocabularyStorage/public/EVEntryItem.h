// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVVocabularyTypes.h"
#include "EVVocabularyInteractionTypes.h"
#include "EVEntryItem.generated.h"

/**
 * Identifies which normalized payload a UEVEntryItem currently carries.
 */
UENUM(BlueprintType)
enum class EEVEntryItemPayloadType : uint8
{
    VocabularyRecord,
    VocabularyMeaning
};

/**
 * Generic UObject payload used by vocabulary ListViews.
 *
 * It can carry:
 * - one complete vocabulary record;
 * - one meaning belonging to a vocabulary record.
 *
 * The legacy flat EntryItem field remains temporarily so the existing
 * ReviewWords flow continues compiling until it is migrated.
 */
UCLASS()
class ENHANCEVOCABULARYSTORAGE_API UEVEntryItem : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Describes which normalized payload should be consumed.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocabulary Entry Item")
    EEVEntryItemPayloadType PayloadType = EEVEntryItemPayloadType::VocabularyRecord;

    /**
     * Complete normalized vocabulary record.
     *
     * Used by ListViews whose entries represent complete words.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocabulary Entry Item")
    FEVVocabularyRecord VocabularyRecord;

    /**
     * One normalized meaning.
     *
     * Used by meaning ListViews inside Search Results,
     * Review Words and Detailed View.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocabulary Entry Item")
    FEVVocabularyMeaning VocabularyMeaning;

    /** Index of VocabularyMeaning inside its owning FEVVocabularyRecord. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocabulary Entry Item")
    int32 VocabularyMeaningIndex = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocabulary Entry Item")
    EEVVocabularyMeaningWidgetMode MeaningWidgetMode = EEVVocabularyMeaningWidgetMode::DetailedReadOnly;

    /**
     * Temporary compatibility payload for the current flat ReviewWords UI.
     *
     * Remove only after ReviewWords and its current UEVWordEntryWidget
     * have been migrated to FEVVocabularyRecord.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocabulary Entry Item|Legacy")
    FVocabularyEntry EntryItem;
};