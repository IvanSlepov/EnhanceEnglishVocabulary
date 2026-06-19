// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "EVReviewWordsWidget.generated.h"

/**
 *
 */
UCLASS()
class ENHANCEVOCABULARYUI_API UEVReviewWordsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UListView* ListView_ReviewWords;

    class UEVGameInstance* EVGameInstance;

    void DisplayWords();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NumberOfWordsToDisplay = 20;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
};
