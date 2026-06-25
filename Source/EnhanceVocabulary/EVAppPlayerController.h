// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "EVAppPlayerController.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWidgetsError, const FEVErrorInfo&, ErrorInfo);

UCLASS()
class ENHANCEVOCABULARY_API AEVAppPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AEVAppPlayerController();

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> RootWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> RootWidgetInstance;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> ErrorWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> ErrorWidgetInstance;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "PC Events")
    FOnWidgetsError OnWidgetsError;

protected:
    virtual void BeginPlay() override;
    void InitEVAppPlayerController();

private:
    UFUNCTION()
    void HandleWidgetErrors(const FEVErrorInfo& WidgetErrorInfo);
};
