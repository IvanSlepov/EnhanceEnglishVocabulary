// Fill out your copyright notice in the Description page of Project Settings.

#include "EVImportExportDBWidget.h"

void UEVImportExportDBWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (WBP_SelectImportExportOptions)
    {
        WBP_SelectImportExportOptions->OnFileOperationSelected.AddDynamic(
            this, &ThisClass::HandleOnImportExportDownloadDBIssued);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("WBP_SelectImportExportOptions is nullptr in EVImportExportDBWidget.cpp"));
    }
}

void UEVImportExportDBWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UEVImportExportDBWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UEVImportExportDBWidget::HandleOnImportExportDownloadDBIssued(
    const FEVFileOperationInfo& FileOperationInfoFromSelectorWidget)
{
    OnImportExportDownloadDBIssued.Broadcast(FileOperationInfoFromSelectorWidget);
}
