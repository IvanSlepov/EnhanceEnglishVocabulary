// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Engine/TimerHandle.h"
#include "EVDisplayStatusProvider.h"
#include "EVRequestedActionStatusWidget.generated.h"

/**
 *
 */
UCLASS()
class ENHANCEVOCABULARYUI_API UEVRequestedActionStatusWidget : public UUserWidget, public IEVDisplayStatusProvider
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_RequestedActionStatus;

    virtual void ShowStatus(const FEVRequestedActionInfo& RequestedActionInfo) override;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    UFUNCTION()
    void HandleRemoveFromParent();

    FTimerHandle AutoCloseTimerHandler;

    float CloseThisIntervalSeconds = 2.f;
};
