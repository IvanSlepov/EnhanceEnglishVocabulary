// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EVGameInstance.generated.h"

class UEVVocabularyStorageService;
class UEVWordSearchService;

/**
 *
 */

UENUM()
enum class EEVVocabularyStorageServiceResult : uint8
{
    WordExists,
    WordDoesNotExist,
    DatabaseError,
    VocabularyStorageInstanceError,
    Empty
};

UCLASS()
class ENHANCEVOCABULARY_API UEVGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TObjectPtr<UEVVocabularyStorageService> VocabularyStorageService;

    UPROPERTY()
    TObjectPtr<UEVWordSearchService> WordSearchService;

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    EEVVocabularyStorageServiceResult DoesWordExist(const FString& Word, FText& OutErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    bool SaveVocabularyEntry(const FWordSearchResult& WordSearchResult);

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    bool GetVocabularyEntries(TArray<FVocabularyEntry>& OutVocabularyEntries, int32 EntryNumber = 5);

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Search")
    FWordSearchResult SearchWordFake(const FString& Word);

private:
    virtual void Init() override;
    virtual void Shutdown() override;
};
