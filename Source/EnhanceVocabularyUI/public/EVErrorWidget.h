// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "EVErrorDisplayWidget.h"
#include "EVErrorWidget.generated.h"

/**
 *
 */

UCLASS()
class ENHANCEVOCABULARYUI_API UEVErrorWidget : public UUserWidget, public IEVErrorDisplayWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_Error;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_Close;

    virtual void ShowError(const FText& ErrorText) override;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    UFUNCTION()
    void HandleOnCloseButtonPressed();
};
