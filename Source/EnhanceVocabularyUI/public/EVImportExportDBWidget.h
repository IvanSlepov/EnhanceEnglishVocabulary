// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVSelectImpExpDBOptionsWidget.h"
#include "EVFileExchangeTypes.h"
#include "EVFeatureRoles.h"
#include "EVImportExportDBWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnImportExportDownloadDBIssued, const FEVFileOperationInfo&,
                                            FileOperationInfo);

/**
 *
 */
UCLASS()
class ENHANCEVOCABULARYUI_API UEVImportExportDBWidget : public UUserWidget, public IEVFileExchangeFeatureRole
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UEVSelectImpExpDBOptionsWidget> WBP_SelectImportExportOptions;

    UPROPERTY(BlueprintCallable)
    FOnImportExportDownloadDBIssued OnImportExportDownloadDBIssued;

    virtual FOnEVFeatureFileOperationRequested& GetFileOperationRequestedEvent() override
    {
        return OnFeatureFileOperationRequested;
    }

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    FOnEVFeatureFileOperationRequested OnFeatureFileOperationRequested;

    UFUNCTION()
    void HandleOnImportExportDownloadDBIssued(const FEVFileOperationInfo& FileOperationInfoFromSelectorWidget);
};
