// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVVocabularyTypes.h"
#include "EVEntryItem.generated.h"

/**
 *
 */
UCLASS()
class ENHANCEVOCABULARYSTORAGE_API UEVEntryItem : public UObject
{
    GENERATED_BODY()

public:
    FVocabularyEntry EntryItem;
};
