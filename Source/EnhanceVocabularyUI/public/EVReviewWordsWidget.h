// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "EVEntryItem.h"
#include "EVWordEntryActionTypes.h"
#include "EVReviewWordsWidget.generated.h"

class UEVWordEntryWidget;
class UUserWidget;

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWordEntryWidgetControlsButtonPressed, const FEVWordEntryActionInfo&,
                                            WordEntryActionInfo);

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
    int32 NumberOfWordsToDisplay = 1000;

    UPROPERTY(BlueprintAssignable)
    FOnWordEntryWidgetControlsButtonPressed OnWordEntryWidgetControlsButtonPressed;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    UPROPERTY()
    TObjectPtr<UEVWordEntryWidget> CurrentlyExpandedEntryWidget;

    void HandleListEntryWidgetGenerated(UUserWidget& Widget);

    FEVWordEntryActionInfo EVWordEntryActionInfo;

    UFUNCTION()
    void HandleWordEntryControlsButtonPressed(UEVWordEntryWidget* WordEntryWidget, bool bViewPressed, bool bEditPressed,
                                              bool bDeletePressed, bool bOkPressed);
};
