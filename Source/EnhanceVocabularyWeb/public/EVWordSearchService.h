// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVVocabularyTypes.h"
#include "EVWordSearchService.generated.h"

/**
 *
 */
UCLASS()
class ENHANCEVOCABULARYWEB_API UEVWordSearchService : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Word Search")
    FWordSearchResult SearchWordFake(const FString& Word);
};
