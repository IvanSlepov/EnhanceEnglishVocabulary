// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EVHelpers.generated.h"

/**
 *
 */
UCLASS()
class ENHANCEVOCABULARYCORE_API UEVHelpers : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // We use this path to create a .db file to use while in Editor
    UFUNCTION(BlueprintCallable, Category = "EV|Paths")
    static FString GetVocabularyDebugDbPath();

    // We use this path to create a template .db file whithin the packaged app
    UFUNCTION(BlueprintCallable, Category = "EV|Paths")
    static FString GetVocabularyCleanDbPath();

    // We use this path in the packaged app to allow CRUD operations
    UFUNCTION(BlueprintCallable, Category = "EV|Paths")
    static FString GetVocabularyLiveDbPath();
};