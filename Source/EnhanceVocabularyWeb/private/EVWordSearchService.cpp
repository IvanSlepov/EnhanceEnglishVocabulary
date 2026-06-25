// Fill out your copyright notice in the Description page of Project Settings.

#include "EVWordSearchService.h"

#include "EVHttpService.h"
#include "EVResponseParser.h"
#include "EVWebProviders.h"

void UEVWordSearchService::Initialize()
{
    HttpService = NewObject<UEVHttpService>(this);
}

FWordSearchResult UEVWordSearchService::SearchWordFake(const FString& Word)
{
    FWordSearchResult Result;

    Result.Word = Word;
    Result.Definition = FString::Printf(TEXT("Fake definition for: %s"), *Word);
    Result.Usage = FString::Printf(TEXT("Fake usage example for: %s"), *Word);
    Result.TranslationRu = TEXT("Fake Russian translation");
    Result.TranslationUa = TEXT("Fake Ukrainian translation");
    Result.bSuccess = true;

    return Result;
}

void UEVWordSearchService::SearchWordOnline(const FString& Word)
{
    if (!HttpService)
    {
        UE_LOG(LogTemp, Error, TEXT("HttpService is null"));
        return;
    }

    if (Word.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("SearchWordOnline: Word is empty"));
        return;
    }

    ResetPendingSearch(Word);

    SendDictionaryRequest(Word);
    SendTranslationRequest(Word, TEXT("ru"));
    SendTranslationRequest(Word, TEXT("uk"));
}

void UEVWordSearchService::SendDictionaryRequest(const FString& Word)
{
    const FString Url = FString(EVWebProviders::FreeDictionary) + Word;

    HttpService->SendGetRequest(
        Url, FEVHttpResponseDelegate::CreateUObject(this, &UEVWordSearchService::HandleDictionaryResponse));

    UE_LOG(LogTemp, Warning, TEXT("Dictionary request sent: %s"), *Url);
}

void UEVWordSearchService::SendTranslationRequest(const FString& Word, const FString& TranslateTo)
{
    const FString Url = FString(EVWebProviders::MyMemory) + TEXT("?q=") + Word + TEXT("&langpair=en|") + TranslateTo;

    if (TranslateTo.Equals(TEXT("ru")))
    {
        HttpService->SendGetRequest(
            Url, FEVHttpResponseDelegate::CreateUObject(this, &UEVWordSearchService::HandleTranslationRuResponse));
    }
    else
    {
        HttpService->SendGetRequest(
            Url, FEVHttpResponseDelegate::CreateUObject(this, &UEVWordSearchService::HandleTranslationUkResponse));
    }

    UE_LOG(LogTemp, Warning, TEXT("Translation request sent: %s"), *Url);
}

void UEVWordSearchService::HandleDictionaryResponse(bool bSuccess, int32 ResponseCode, const FString& ResponseBody)
{
    if (!bSuccess || ResponseCode != 200)
    {
        PendingResult.bSuccess = false;
        OnEVWordSearchCompleted.Broadcast(PendingResult);
        return;
    }

    FWordSearchResult ParsedResult;

    if (!FEVResponseParser::ParseFreeDictionaryResponse(ResponseBody, ParsedResult))
    {
        PendingResult.bSuccess = false;
        OnEVWordSearchCompleted.Broadcast(PendingResult);
        return;
    }

    PendingResult.Word = ParsedResult.Word;
    PendingResult.Definition = ParsedResult.Definition;
    PendingResult.Usage = ParsedResult.Usage;

    bDictionaryCompleted = true;

    TryCompleteSearch();
}

void UEVWordSearchService::HandleTranslationRuResponse(bool bSuccess, int32 ResponseCode, const FString& ResponseBody)
{
    FString Translation;

    if (bSuccess && ResponseCode == 200 &&
        FEVResponseParser::ParseMyMemoryTranslationResponse(ResponseBody, Translation))
    {
        PendingResult.TranslationRu = Translation;
    }

    bTranslationRuCompleted = true;

    TryCompleteSearch();
}

void UEVWordSearchService::HandleTranslationUkResponse(bool bSuccess, int32 ResponseCode, const FString& ResponseBody)
{
    FString Translation;

    if (bSuccess && ResponseCode == 200 &&
        FEVResponseParser::ParseMyMemoryTranslationResponse(ResponseBody, Translation))
    {
        PendingResult.TranslationUa = Translation;
    }

    bTranslationUkCompleted = true;

    TryCompleteSearch();
}

void UEVWordSearchService::ResetPendingSearch(const FString& Word)
{
    PendingResult = FWordSearchResult();

    PendingResult.Word = Word;

    CurrentSearchWord = Word;

    bDictionaryCompleted = false;
    bTranslationRuCompleted = false;
    bTranslationUkCompleted = false;
}

void UEVWordSearchService::TryCompleteSearch()
{
    if (!bDictionaryCompleted || !bTranslationRuCompleted || !bTranslationUkCompleted)
    {
        return;
    }

    PendingResult.bSuccess = true;

    OnEVWordSearchCompleted.Broadcast(PendingResult);
}