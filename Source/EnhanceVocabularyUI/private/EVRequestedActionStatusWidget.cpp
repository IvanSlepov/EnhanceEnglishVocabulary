// Fill out your copyright notice in the Description page of Project Settings.

#include "EVRequestedActionStatusWidget.h"

void UEVRequestedActionStatusWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
}

void UEVRequestedActionStatusWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UEVRequestedActionStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UWorld* World = GetWorld())
    {

        World->GetTimerManager().SetTimer(AutoCloseTimerHandler, this, &ThisClass::HandleRemoveFromParent,
                                          CloseThisIntervalSeconds, true);
    }
}

void UEVRequestedActionStatusWidget::HandleRemoveFromParent()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoCloseTimerHandler);
    }
    RemoveFromParent();
}

void UEVRequestedActionStatusWidget::ShowStatus(const FEVRequestedActionInfo& RequestedActionInfo)
{
    if (TextBlock_RequestedActionStatus)
    {
        TextBlock_RequestedActionStatus->SetText(RequestedActionInfo.Message);
        TextBlock_RequestedActionStatus->SetColorAndOpacity(FSlateColor(RequestedActionInfo.Color));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("The TextBlock_RequestedActionStatus is nullptr."));
    }
}