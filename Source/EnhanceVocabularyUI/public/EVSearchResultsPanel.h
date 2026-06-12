// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "EVSearchResultsPanel.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSavePressed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDiscardPressed);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVSearchResultsPanel : public UUserWidget
{
    GENERATED_BODY()

public:
    // Widget elements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_SearchResultsDefinition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_SearchResultsUsage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_SearchResultsTranslation_Russian;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_SearchResultsTranslation_Ukrainian;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_Save;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_Discard;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Search Results Panel Button Events")
    FOnSavePressed OnSavePressed;

    UPROPERTY(BlueprintAssignable, Category = "Search Results Panel Button Events")
    FOnDiscardPressed OnDiscardPressed;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

private:
    UFUNCTION()
    void HandleSavePressed();

    UFUNCTION()
    void HandleDiscardPressed();
};
