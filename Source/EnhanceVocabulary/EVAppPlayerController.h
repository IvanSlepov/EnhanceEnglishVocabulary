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
#include "EVWordEntryDisplayWidgetProvider.h"
#include "EVGameInstance.h"
#include "EVWidgetCommonEvents.h"
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

    IEVWidgetCommonEvents* WidgetCommonEvents = nullptr;

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

    IEVWordEntryDisplayWidgetProvider* DetailedWordEntryDisplay = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> ConfirmationDialogWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> ConfirmationDialogWidgetInstance;

    IEVConfirmationDialogWidgetProvider* ConfirmationDialogWidget = nullptr;

    class UEVGameInstance* EVGameInstance;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "PC Events")
    FOnWidgetsError OnWidgetsError;

protected:
    virtual void BeginPlay() override;
    void InitEVAppPlayerController();

private:
    // Cache the data we receive from the the WordEntry
    // we decided to review
    FEVWordEntryActionInfo CachedWordEntryWidgetInfo;

    // Cache the result we want to display AFTER
    // the word has been edited. If we discard changes
    // we'll retreat to displaying FEVWordEntryActionInfo CachedWordEntryWidgetInfo
    FVocabularyEntry CachedConfirmedWordEntry;

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
    void HandleDetailedEditButtonPressed();

    UFUNCTION()
    void HandleDetailedDeleteButtonPressed();

    UFUNCTION()
    void HandleDetailedSaveChangesButtonPressed(const FVocabularyEntry& NewVocabularyEntry);

    // Handlers for the "EVConfirmationDialogWidget.h"
    UFUNCTION()
    void HandleCreateConfirmationDialog(EEVConfirmationDialogType DialogType, EEVWordEntryActionType PendingActionType);

    UFUNCTION()
    void HandleConfirmationDialog_ButtonPressed(bool bIsOperationConfirmed);

    // Handle Edit word entry
    UFUNCTION()
    void ProcessConfirmedWordUpdate();

    // Handle Delete word entry
    UFUNCTION()
    void ProcessConfirmedWordDelete();

    // Handle widget destruction
    void DestroyWidget(TObjectPtr<UUserWidget>& Widget);
};
