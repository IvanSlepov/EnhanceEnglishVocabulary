// Fill out your copyright notice in the Description page of Project Settings.

#include "EVAppPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "EVRootWidget.h"

AEVAppPlayerController::AEVAppPlayerController()
{
    bShowMouseCursor = false;
    bEnableClickEvents = true;
    bEnableTouchEvents = true;
    bEnableMouseOverEvents = false;
    bEnableTouchOverEvents = false;
}

void AEVAppPlayerController::BeginPlay()
{
    Super::BeginPlay();

    InitEVAppPlayerController();
    VocabularyStorageService = NewObject<UEVVocabularyStorageService>(this);
    WordSearchService = NewObject<UEVWordSearchService>(this);
}

void AEVAppPlayerController::InitEVAppPlayerController()
{
    ActivateTouchInterface(nullptr);

    FInputModeUIOnly InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);

    if (RootWidgetClass)
    {
        RootWidgetInstance = CreateWidget<UUserWidget>(this, RootWidgetClass);

        if (RootWidgetInstance)
        {
            RootWidgetInstance->AddToViewport();
        }
    }
}

bool AEVAppPlayerController::TestSaveVocabularyEntry(const FWordSearchResult& WordSearchResult)
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

FWordSearchResult AEVAppPlayerController::SearchWordFake(const FString& Word)
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
