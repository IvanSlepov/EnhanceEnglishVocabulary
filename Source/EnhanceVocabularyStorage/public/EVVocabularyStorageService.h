// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVVocabularyTypes.h"
#include "EVVocabularyStorageService.generated.h"

/**
 * 
 */
UCLASS()
class ENHANCEVOCABULARYSTORAGE_API UEVVocabularyStorageService : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
	bool SaveVocabularyEntry(const FVocabularyEntry& Entry);
	
};
