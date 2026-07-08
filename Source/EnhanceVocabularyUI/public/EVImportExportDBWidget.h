// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVSelectImpExpDBOptionsWidget.h"
#include "EVImportExportDBWidget.generated.h"

/**
 *
 */
UCLASS()
class ENHANCEVOCABULARYUI_API UEVImportExportDBWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UEVSelectImpExpDBOptionsWidget> WBP_SelectImportExportOptions;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
};
