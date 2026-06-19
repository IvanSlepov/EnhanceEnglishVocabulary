// Fill out your copyright notice in the Description page of Project Settings.

#include "EVErrorWidget.h"

void UEVErrorWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_Close)
    {
        Button_Close->OnPressed.AddDynamic(this, &ThisClass::HandleOnCloseButtonPressed);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_Close is nullptr"));
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

void UEVErrorWidget::HandleOnCloseButtonPressed()
{
    RemoveFromParent();
}

void UEVErrorWidget::ShowError(const FText& ErrorText)
{
    TextBlock_Error->SetText(ErrorText);
}