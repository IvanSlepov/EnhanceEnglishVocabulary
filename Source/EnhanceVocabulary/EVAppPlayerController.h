// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EVVocabularyStorageService.h"
#include "EVAppPlayerController.generated.h"

/**
 * 
 */
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

	UPROPERTY()
	TObjectPtr<UEVVocabularyStorageService> VocabularyStorageService;

	UFUNCTION(BlueprintCallable, Category = "Vocabulary Test")
	void TestSaveVocabularyEntry();

protected:

	virtual void BeginPlay() override;
	void InitEVAppPlayerController();
};
