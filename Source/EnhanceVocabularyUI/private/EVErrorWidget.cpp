// Fill out your copyright notice in the Description page of Project Settings.

#include "EVErrorWidget.h"

void UEVErrorWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_Close)
    {
        Button_Close->OnPressed.AddDynamic(this, &ThisClass::HandleRemoveFromParent);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_Close is nullptr"));
    }

    if (UWorld* World = GetWorld())
    {

        World->GetTimerManager().SetTimer(AutoCloseTimerHandler, this, &UEVErrorWidget::HandleRemoveFromParent,
                                          CloseThisIntervalSeconds, true);
    }
}

void UEVErrorWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UEVErrorWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UEVErrorWidget::HandleRemoveFromParent()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoCloseTimerHandler);
    }
    RemoveFromParent();
}

void UEVErrorWidget::ShowError(const FText& ErrorText)
{
    TextBlock_Error->SetText(ErrorText);
}