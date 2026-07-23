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

// USE OnClicked cause OnPressed may cause stale issues on Mobile
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveClicked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDiscardClicked);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVSearchResultsPanel : public UUserWidget
{
    GENERATED_BODY()

public:
    // Widget elements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_SearchResultsTranscription;

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
    FOnSaveClicked OnSaveClicked;

    UPROPERTY(BlueprintAssignable, Category = "Search Results Panel Button Events")
    FOnDiscardClicked OnDiscardClicked;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

private:
    UFUNCTION()
    void HandleSavePressed();

    UFUNCTION()
    void HandleDiscardPressed();
};
