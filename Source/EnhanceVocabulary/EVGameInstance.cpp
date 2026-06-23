// Fill out your copyright notice in the Description page of Project Settings.

#include "EVGameInstance.h"

#include "EVVocabularyStorageService.h"
#include "EVWordSearchService.h"
#include "EVConnectivityService.h"

void UEVGameInstance::Init()
{
    Super::Init();

    VocabularyStorageService = NewObject<UEVVocabularyStorageService>(this);
    WordSearchService = NewObject<UEVWordSearchService>(this);

    ConnectivityService = NewObject<UEVConnectivityService>(this);

    if (ConnectivityService)
    {
        ConnectivityService->Initialize();
        ConnectivityService->RefreshConnection();
        ConnectivityService->OnConnectionStateChanged.AddDynamic(this, &UEVGameInstance::HandleConnectionStateChanged);

        if (UWorld* World = GetWorld())
        {
            ConnectivityService->StartConnectionPolling(World);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("World is null; connection polling not started"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ConnectivityService is null"));
    }

    if (VocabularyStorageService)
    {
        VocabularyStorageService->InitializeStorage();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));
    }
}

void UEVGameInstance::Shutdown()
{
    if (VocabularyStorageService)
    {
        VocabularyStorageService->ShutdownStorage();
    }

    if (ConnectivityService)
    {
        if (UWorld* World = GetWorld())
        {
            ConnectivityService->StopConnectionPolling(World);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("World is null; connection polling not stopped"));
        }

        ConnectivityService->Shutdown();
    }

    Super::Shutdown();
}

EEVVocabularyStorageServiceResult UEVGameInstance::DoesWordExist(const FString& Word, FText& OutErrorMessage)
{
    OutErrorMessage = FText::GetEmpty();

    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));
        return EEVVocabularyStorageServiceResult::VocabularyStorageInstanceError;
    }

    switch (VocabularyStorageService->DoesWordExist(Word, OutErrorMessage))
    {
    case EEVWordLookupResult::DatabaseError:
        return EEVVocabularyStorageServiceResult::DatabaseError;
        break;
    case EEVWordLookupResult::Exists:
        return EEVVocabularyStorageServiceResult::WordExists;
        break;
    case EEVWordLookupResult::DoesNotExist:
        return EEVVocabularyStorageServiceResult::WordDoesNotExist;
        break;
    default:
        break;
    }

    return EEVVocabularyStorageServiceResult::Empty;
}

bool UEVGameInstance::SaveVocabularyEntry(const FWordSearchResult& WordSearchResult)
{
    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));
        return false;
    }

    FVocabularyEntry Entry;
    Entry.Word = WordSearchResult.Word;
    Entry.Definition = WordSearchResult.Definition;
    Entry.Usage = WordSearchResult.Usage;
    Entry.TranslationRu = WordSearchResult.TranslationRu;
    Entry.TranslationUa = WordSearchResult.TranslationUa;

    if (VocabularyStorageService->SaveVocabularyEntry(Entry))
    {
        return true;
    }

    return false;
}

bool UEVGameInstance::GetVocabularyEntries(TArray<FVocabularyEntry>& OutVocabularyEntries, int32 EntryNumber)
{
    if (VocabularyStorageService)
    {
        OutVocabularyEntries = VocabularyStorageService->GetVocabularyEntries(EntryNumber);
        return true;
    }

    UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));
    return false;
}

FWordSearchResult UEVGameInstance::SearchWordFake(const FString& Word)
{
    if (!WordSearchService)
    {
        FWordSearchResult Result;
        Result.bSuccess = false;
        Result.ErrorMessage = TEXT("WordSearchService is null");
        return Result;
    }

    return WordSearchService->SearchWordFake(Word);
}

EEVConnectionState UEVGameInstance::GetConnectionState() const
{
    if (ConnectivityService)
    {
        return ConnectivityService->GetConnectionState();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ConnectivityService is null in UEVGameInstance::GetConnectionState()"));
        return EEVConnectionState::Offline;
    }
}

void UEVGameInstance::HandleConnectionStateChanged(EEVConnectionState NewState)
{
    if (EVConnectionState == NewState)
    {
        return;
    }

    EVConnectionState = NewState;
    OnConnectionStateChanged.Broadcast(NewState);

    if (EVConnectionState == EEVConnectionState::Offline)
    {
        FEVErrorInfo EVErrorInfo;

        EVErrorInfo.Source = EEVErrorSource::ConnectionModule;
        EVErrorInfo.Type = EEVErrorType::ConnectionError;
        EVErrorInfo.Message = FText::FromString(TEXT("Failed to connect to the WEB. You are Offline."));

        OnConnectionError.Broadcast(EVErrorInfo);
    }
}