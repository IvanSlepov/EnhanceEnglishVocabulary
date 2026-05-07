// Fill out your copyright notice in the Description page of Project Settings.


#include "EVAppPlayerController.h"

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

	ActivateTouchInterface(nullptr);

	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
}

