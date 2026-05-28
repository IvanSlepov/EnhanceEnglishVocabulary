// Fill out your copyright notice in the Description page of Project Settings.

#include "EVWordSearchService.h"

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
