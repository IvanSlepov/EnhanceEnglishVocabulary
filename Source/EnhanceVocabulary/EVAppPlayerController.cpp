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

void AEVAppPlayerController::TestSaveVocabularyEntry()
{
	if (!VocabularyStorageService)
	{
		UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));
		return;
	}

	FVocabularyEntry Entry;
	Entry.Word = TEXT("test");
	Entry.Definition = TEXT("test definition");
	Entry.TranslationRu = TEXT("test");
	Entry.TranslationUa = TEXT("test");

	VocabularyStorageService->SaveVocabularyEntry(Entry);
}
