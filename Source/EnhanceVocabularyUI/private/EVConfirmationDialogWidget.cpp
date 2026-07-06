// Fill out your copyright notice in the Description page of Project Settings.

#include "EVConfirmationDialogWidget.h"

void UEVConfirmationDialogWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_Confirm && Button_Discard)
    {
        Button_Confirm->OnPressed.AddDynamic(this, &ThisClass::HandleConfirmPressed);
        Button_Discard->OnPressed.AddDynamic(this, &ThisClass::HandleDiscardPressed);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Some/both Confirmation dialog buttons are nullptr."));
    }
}

void UEVConfirmationDialogWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UEVConfirmationDialogWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UEVConfirmationDialogWidget::SetConfirmationDialogInfo(const FEVConfirmationDialogInfo& EVConfirmationDialogInfo)
{
    TextBlock_ConfirmationDialog->SetText(EVConfirmationDialogInfo.DialogText);
    TextBlock_ConfirmationDialog->SetColorAndOpacity(EVConfirmationDialogInfo.DialogTextColor);

    TextBlock_Confirm->SetText(EVConfirmationDialogInfo.ConfirmButtonText);
    TextBlock_Confirm->SetColorAndOpacity(EVConfirmationDialogInfo.ConfirmButtonColor);

    TextBlock_Discard->SetText(EVConfirmationDialogInfo.DiscardButtonText);
    TextBlock_Discard->SetColorAndOpacity(EVConfirmationDialogInfo.DiscardButtonColor);
}

void UEVConfirmationDialogWidget::HandleConfirmPressed()
{
    OnConfirmationDialogButtonPressed.Broadcast(true);
}

void UEVConfirmationDialogWidget::HandleDiscardPressed()
{
    OnConfirmationDialogButtonPressed.Broadcast(false);
}