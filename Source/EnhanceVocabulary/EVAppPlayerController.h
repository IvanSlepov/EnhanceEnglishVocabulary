// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
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

protected:

	virtual void BeginPlay() override;
};
