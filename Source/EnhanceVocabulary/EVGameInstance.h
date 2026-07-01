// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EVConnectionTypesAndEnums.h"
#include "EVWebProviderTypes.h"

#include "EVGameInstance.generated.h"

class UEVVocabularyStorageService;
class UEVWordSearchService;
class UEVConnectivityService;

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEVConnectionStateChanged, EEVConnectionState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEVWordSearchCompletedFromEVGameInstance, const FWordSearchResult&, Result);

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

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Fake/Debugging Search")
    FWordSearchResult SearchWordFake(const FString& Word);

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Online Search")
    void SearchWordOnline(const FString& Word, EEVWebProvider DefinitionUsageProvider,
                          EEVWebProvider TranslationProvider);

    // Connection functions
    EEVConnectionState GetConnectionState() const;

    /*Events*/
    UPROPERTY(BlueprintAssignable)
    FEVConnectionStateChanged OnConnectionStateChanged;

    UPROPERTY(BlueprintAssignable)
    FEVWordSearchCompletedFromEVGameInstance OnEVWordSearchCompletedFromEVGameInstance;

protected:
    virtual void Init() override;
    virtual void Shutdown() override;

private:
    UFUNCTION()
    void HandleConnectionStateChanged(EEVConnectionState NewState);

    UFUNCTION()
    void HandleEVWordSearchCompletedFromEVGameInstance(const FWordSearchResult& SearchWordResultPassedByGameInstance);

    // We need to assign local ENUM var (Cause unassigned enum type var takes the very first entry from that enum)
    // and we need to assign smth that differ from the Offline in this case, to address the issue when
    // the app is launched with already disabled Internet access but the corresponding error message never appears
    // since the even responsible for that action never fires at launch due to local var being Offline and
    // the connectivity var being set to Offline and that makes the check in the corresponding Handler to return
    // immediately
    EEVConnectionState EVConnectionState = EEVConnectionState::Connecting;
};
