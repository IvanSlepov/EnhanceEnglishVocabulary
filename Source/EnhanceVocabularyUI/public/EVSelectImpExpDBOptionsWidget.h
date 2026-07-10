// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "EVFileExchangeTypes.h"
#include "EVSelectImpExpDBOptionsWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFileOperationSelected, const FEVFileOperationInfo&, FileOperationInfo);

/**
 *
 */
UCLASS()
class ENHANCEVOCABULARYUI_API UEVSelectImpExpDBOptionsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_ImportExportRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UComboBoxString* ComboBoxString_DownloadDBTemplate_Options;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_DownloadDBTemplate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UComboBoxString* ComboBoxString_ImportDB_Options;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_StartDBImport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UComboBoxString* ComboBoxString_ExportDB_Options;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_StartDBExport;

    UPROPERTY(BlueprintAssignable)
    FOnFileOperationSelected OnFileOperationSelected;

public:
    FEVFileOperationInfo GetImportDBExtensionType();
    FEVFileOperationInfo GetExportDBExtensionType();
    FEVFileOperationInfo GetDownloadDBTemplateExtensionType();

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    void PopulateFileExtensionOptionsComboBoxes();

    FString FileExtensionNameEnumToString(EEVFileExtensionType FileExtensionType) const;
    EEVFileExtensionType FileExtensionNameStringToEnum(const FString& FileExtensionName) const;

    UFUNCTION()
    void HandleDownloadDBTemplate();

    UFUNCTION()
    void HandleExportDB();
};
