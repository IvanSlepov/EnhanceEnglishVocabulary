// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EVConnectionTypesAndEnums.h"

#include "EVGameInstance.generated.h"

class UEVVocabularyStorageService;
class UEVWordSearchService;
class UEVConnectivityService;

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEVConnectionStateChanged, EEVConnectionState, NewState);

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

    UPROPERTY()
    TObjectPtr<UEVConnectivityService> ConnectivityService;

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    EEVVocabularyStorageServiceResult DoesWordExist(const FString& Word, FText& OutErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    bool SaveVocabularyEntry(const FWordSearchResult& WordSearchResult);

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    bool GetVocabularyEntries(TArray<FVocabularyEntry>& OutVocabularyEntries, int32 EntryNumber = 5);

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Search")
    FWordSearchResult SearchWordFake(const FString& Word);

    // Connection functions
    EEVConnectionState GetConnectionState() const;

    // Events
    UPROPERTY(BlueprintAssignable)
    FEVConnectionStateChanged OnConnectionStateChanged;

protected:
    virtual void Init() override;
    virtual void Shutdown() override;

private:
    UFUNCTION()
    void HandleConnectionStateChanged(EEVConnectionState NewState);

    EEVConnectionState EVConnectionState;
};
