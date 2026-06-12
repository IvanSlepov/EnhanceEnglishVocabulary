// Fill out your copyright notice in the Description page of Project Settings.

#include "EVSearchResultsPanel.h"

void UEVSearchResultsPanel::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_Save)
    {
        Button_Save->OnPressed.AddDynamic(this, &UEVSearchResultsPanel::HandleSavePressed);
    }

    if (Button_Discard)
    {
        Button_Discard->OnPressed.AddDynamic(this, &UEVSearchResultsPanel::HandleDiscardPressed);
    }
}

void UEVSearchResultsPanel::NativeConstruct()
{
    Super::NativeConstruct();
}

void UEVSearchResultsPanel::HandleSavePressed()
{
    OnSavePressed.Broadcast();
    UE_LOG(LogTemp, Warning, TEXT("Save pressed"));
}

void UEVSearchResultsPanel::HandleDiscardPressed()
{
    UE_LOG(LogTemp, Warning, TEXT("Discard pressed"));
    OnDiscardPressed.Broadcast();
}
