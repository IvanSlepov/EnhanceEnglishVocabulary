// Fill out your copyright notice in the Description page of Project Settings.

#include "EVSearchResultsPanel.h"

void UEVSearchResultsPanel::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_Save)
    {
        Button_Save->OnClicked.AddDynamic(this, &UEVSearchResultsPanel::HandleSavePressed);
    }

    if (Button_Discard)
    {
        Button_Discard->OnClicked.AddDynamic(this, &UEVSearchResultsPanel::HandleDiscardPressed);
    }
}

void UEVSearchResultsPanel::NativeConstruct()
{
    Super::NativeConstruct();

    Button_Save->SetIsEnabled(true);
    Button_Discard->SetIsEnabled(true);
}

void UEVSearchResultsPanel::HandleSavePressed()
{
    OnSaveClicked.Broadcast();
    UE_LOG(LogTemp, Warning, TEXT("Save pressed"));
}

void UEVSearchResultsPanel::HandleDiscardPressed()
{
    UE_LOG(LogTemp, Warning, TEXT("Discard pressed"));
    OnDiscardClicked.Broadcast();
}
