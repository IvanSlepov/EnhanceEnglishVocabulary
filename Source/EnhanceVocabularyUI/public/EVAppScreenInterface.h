// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVAppScreenInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEVAppScreenInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 *
 */
class ENHANCEVOCABULARYUI_API IEVAppScreenInterface
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "App Screen")
    void OnScreenShown();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "App Screen")
    void OnScreenHidden();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "App Screen")
    void RefreshScreen();
};
