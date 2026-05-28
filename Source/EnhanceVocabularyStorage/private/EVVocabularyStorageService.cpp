// Fill out your copyright notice in the Description page of Project Settings.


#include "EVVocabularyStorageService.h"

bool UEVVocabularyStorageService::SaveVocabularyEntry(const FVocabularyEntry& Entry)
{
    UE_LOG(LogTemp, Warning, TEXT("Saving word: %s"), *Entry.Word);

    if (GEngine) 
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Word: %s"), *Entry.Word));
    }

    return true;
}
