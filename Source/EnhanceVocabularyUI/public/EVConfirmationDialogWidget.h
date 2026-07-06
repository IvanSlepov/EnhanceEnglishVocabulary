// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "EVWordEntryActionTypes.h"
#include "EVRequestedActionTypes.h"
#include "EVConfirmationDialogWidgetProvider.h"
#include "EVConfirmationDialogWidget.generated.h"

/**
 *
 */
UCLASS()
class ENHANCEVOCABULARYUI_API UEVConfirmationDialogWidget : public UUserWidget,
                                                            public IEVConfirmationDialogWidgetProvider
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_ConfirmationDialog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_Confirm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_Confirm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_Discard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_Discard;

    virtual void SetConfirmationDialogInfo(const FEVConfirmationDialogInfo& EVConfirmationDialogInfo) override;

    /*Events*/

    // The following events are used to allow PC
    // to bind to this widget button events
    FOnConfirmationDialogButtonPressed OnConfirmationDialogButtonPressed;

    virtual FOnConfirmationDialogButtonPressed& GetDialogButtonPressedDelegate() override
    {
        return OnConfirmationDialogButtonPressed;
    }

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativePreConstruct() override;

private:
    UFUNCTION()
    void HandleConfirmPressed();

    UFUNCTION()
    void HandleDiscardPressed();
};
