// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "EVRequestedActionTypes.h"
#include "EVErrorTypes.h"
#include "EVWordEntryActionTypes.h"
#include "EVConfirmationDialogActionTypes.h"
#include "EVConfirmationDialogWidgetProvider.h"
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

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> LoadingSpinnerClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> LoadingSpinnerInstance;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> RequestedActionStatusWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> RequestedActionStatusWidgetInstance;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> DetailedWordEntryWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> DetailedWordEntryWidgetInstance;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> ConfirmationDialogWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> ConfirmationDialogWidgetInstance;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "PC Events")
    FOnWidgetsError OnWidgetsError;

protected:
    virtual void BeginPlay() override;
    void InitEVAppPlayerController();

private:
    UFUNCTION()
    void HandleWidgetErrors(const FEVErrorInfo& WidgetErrorInfo);

    UFUNCTION()
    void HandleLoadingSpinner(bool bDisplayLoadingSpinner);

    UFUNCTION()
    void HandleActionStatusWidget(const FEVRequestedActionInfo& RequestedActionInfo);

    UFUNCTION()
    void HandleWordEntryWidget(const FEVWordEntryActionInfo& CurrentWordEntryWidgetInfo);

    // Handlers for the "EVWordEntryWidgetDetailed.h" buttons
    UFUNCTION()
    void HandleDetailedViewButtonPressed();

    UFUNCTION()
    void HandleCreateConfirmationDialog();

    UFUNCTION()
    void HandleConfirmationDialog_ButtonPressed(bool bIsOperationConfirmed);
};
