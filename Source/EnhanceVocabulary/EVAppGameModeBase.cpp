// Fill out your copyright notice in the Description page of Project Settings.


#include "EVAppGameModeBase.h"

#include "EVAppPlayerController.h"

AEVAppGameModeBase::AEVAppGameModeBase()
{
	PlayerControllerClass = AEVAppPlayerController::StaticClass();

	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
	SpectatorClass = nullptr;
}
