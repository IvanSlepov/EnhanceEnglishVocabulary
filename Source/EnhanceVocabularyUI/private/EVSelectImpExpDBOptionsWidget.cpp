// Fill out your copyright notice in the Description page of Project Settings.

#include "EVSelectImpExpDBOptionsWidget.h"

void UEVSelectImpExpDBOptionsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_DownloadDBTemplate)
    {
        Button_DownloadDBTemplate->OnPressed.AddDynamic(this, &ThisClass::HandleDownloadDBTemplate);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_DownloadDBTemplate is nullptr in EVSelectImpExpDBOptionsWidget.cpp"));
    }

    if (Button_StartDBExport)
    {
        Button_StartDBExport->OnPressed.AddDynamic(this, &ThisClass::HandleExportDB);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_StartDBExport is nullptr in EVSelectImpExpDBOptionsWidget.cpp"));
    }

    if (Button_StartDBImport_OVERWRITE)
    {
        Button_StartDBImport_OVERWRITE->OnPressed.AddDynamic(this, &ThisClass::HandleImport_OverwriteDB);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_StartDBImport_OVERWRITE is nullptr in EVSelectImpExpDBOptionsWidget.cpp"));
    }

    if (Button_StartDBImport_APPEND)
    {
        Button_StartDBImport_APPEND->OnPressed.AddDynamic(this, &ThisClass::HandleImport_AppendDB);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button_StartDBImport_APPEND is nullptr in EVSelectImpExpDBOptionsWidget.cpp"));
    }
}

void UEVSelectImpExpDBOptionsWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UEVSelectImpExpDBOptionsWidget::NativeConstruct()
{
    Super::NativeConstruct();
    PopulateFileExtensionOptionsComboBoxes();
}

void UEVSelectImpExpDBOptionsWidget::PopulateFileExtensionOptionsComboBoxes()
{
    if (!ComboBoxString_DownloadDBTemplate_Options || !ComboBoxString_ImportDB_OVERWRITE_Options ||
        !ComboBoxString_ImportDB_APPEND_Options || !ComboBoxString_ExportDB_Options)
    {
        UE_LOG(LogTemp, Error, TEXT("File extensions combo boxes are not bound in WBP_SelectImpExpDBOptionsWidget"));
        return;
    }

    ComboBoxString_DownloadDBTemplate_Options->ClearOptions();
    ComboBoxString_ImportDB_OVERWRITE_Options->ClearOptions();
    ComboBoxString_ImportDB_APPEND_Options->ClearOptions();
    ComboBoxString_ExportDB_Options->ClearOptions();

    const TArray<EEVFileExtensionType> AvailableFileExtensions = FEVFileOperationInfo::GetAllAvailableFileExtensions();

    for (const EEVFileExtensionType& FileExtensionType : AvailableFileExtensions)
    {
        const FString ExtensionName = FileExtensionNameEnumToString(FileExtensionType);

        ComboBoxString_DownloadDBTemplate_Options->AddOption(ExtensionName);
        ComboBoxString_ImportDB_OVERWRITE_Options->AddOption(ExtensionName);
        ComboBoxString_ImportDB_APPEND_Options->AddOption(ExtensionName);
        ComboBoxString_ExportDB_Options->AddOption(ExtensionName);
    }

    ComboBoxString_DownloadDBTemplate_Options->SetSelectedOption(
        FileExtensionNameEnumToString(EEVFileExtensionType::Csv));
    ComboBoxString_ImportDB_OVERWRITE_Options->SetSelectedOption(
        FileExtensionNameEnumToString(EEVFileExtensionType::Csv));
    ComboBoxString_ImportDB_APPEND_Options->SetSelectedOption(FileExtensionNameEnumToString(EEVFileExtensionType::Csv));
    ComboBoxString_ExportDB_Options->SetSelectedOption(FileExtensionNameEnumToString(EEVFileExtensionType::Csv));
}

FString UEVSelectImpExpDBOptionsWidget::FileExtensionNameEnumToString(EEVFileExtensionType FileExtensionType) const
{
    const UEnum* EnumPtr = StaticEnum<EEVFileExtensionType>();

    if (!EnumPtr)
    {
        return TEXT("Unknown");
    }

    return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(FileExtensionType)).ToString();
}

EEVFileExtensionType
UEVSelectImpExpDBOptionsWidget::FileExtensionNameStringToEnum(const FString& FileExtensionName) const
{
    const UEnum* EnumPtr = StaticEnum<EEVFileExtensionType>();

    if (!EnumPtr)
    {
        return EEVFileExtensionType::Csv;
    }

    for (const EEVFileExtensionType& FileExtensionType : FEVFileOperationInfo::GetAllAvailableFileExtensions())
    {
        const FString DisplayName =
            EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(FileExtensionType)).ToString();

        if (DisplayName == FileExtensionName)
        {
            return FileExtensionType;
        }
    }

    return EEVFileExtensionType::Csv;
}

void UEVSelectImpExpDBOptionsWidget::HandleDownloadDBTemplate()
{
    OnFileOperationSelected.Broadcast(GetDownloadDBTemplateExtensionType());
}

void UEVSelectImpExpDBOptionsWidget::HandleExportDB()
{
    OnFileOperationSelected.Broadcast(GetExportDBExtensionType());
}

void UEVSelectImpExpDBOptionsWidget::HandleImport_OverwriteDB()
{
    OnFileOperationSelected.Broadcast(GetImportDB_Overwrite_ExtensionType());
}

void UEVSelectImpExpDBOptionsWidget::HandleImport_AppendDB()
{
    OnFileOperationSelected.Broadcast(GetImportDB_Append_ExtensionType());
}

FEVFileOperationInfo UEVSelectImpExpDBOptionsWidget::GetImportDB_Overwrite_ExtensionType()
{
    if (ComboBoxString_ImportDB_OVERWRITE_Options)
    {
        FEVFileOperationInfo EVFileOperationInfo;
        EVFileOperationInfo.OperationType = EEVFileOperationType::ImportDBOverwrite;
        EVFileOperationInfo.FileExtensionType =
            FileExtensionNameStringToEnum(ComboBoxString_ImportDB_OVERWRITE_Options->GetSelectedOption());

        return EVFileOperationInfo;
    }
    else
    {
        UE_LOG(LogTemp, Error,
               TEXT("ComboBoxString_ImportDB_OVERWRITE_Options is nullptr EVSelectImpExpDBOptionsWidget.cpp"));
        return FEVFileOperationInfo();
    }
}

FEVFileOperationInfo UEVSelectImpExpDBOptionsWidget::GetImportDB_Append_ExtensionType()
{
    if (ComboBoxString_ImportDB_APPEND_Options)
    {
        FEVFileOperationInfo EVFileOperationInfo;
        EVFileOperationInfo.OperationType = EEVFileOperationType::ImportDBAppend;
        EVFileOperationInfo.FileExtensionType =
            FileExtensionNameStringToEnum(ComboBoxString_ImportDB_APPEND_Options->GetSelectedOption());

        return EVFileOperationInfo;
    }
    else
    {
        UE_LOG(LogTemp, Error,
               TEXT("ComboBoxString_ImportDB_APPEND_Options is nullptr EVSelectImpExpDBOptionsWidget.cpp"));
        return FEVFileOperationInfo();
    }
}

FEVFileOperationInfo UEVSelectImpExpDBOptionsWidget::GetExportDBExtensionType()
{
    if (ComboBoxString_ExportDB_Options)
    {
        FEVFileOperationInfo EVFileOperationInfo;
        EVFileOperationInfo.OperationType = EEVFileOperationType::ExportDB;
        EVFileOperationInfo.FileExtensionType =
            FileExtensionNameStringToEnum(ComboBoxString_ExportDB_Options->GetSelectedOption());

        return EVFileOperationInfo;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ComboBoxString_ExportDB_Options is nullptr EVSelectImpExpDBOptionsWidget.cpp"));
        return FEVFileOperationInfo();
    }
}

FEVFileOperationInfo UEVSelectImpExpDBOptionsWidget::GetDownloadDBTemplateExtensionType()
{
    if (ComboBoxString_DownloadDBTemplate_Options)
    {
        FEVFileOperationInfo EVFileOperationInfo;
        EVFileOperationInfo.OperationType = EEVFileOperationType::DownloadTemplate;
        EVFileOperationInfo.FileExtensionType =
            FileExtensionNameStringToEnum(ComboBoxString_DownloadDBTemplate_Options->GetSelectedOption());

        return EVFileOperationInfo;
    }
    else
    {
        UE_LOG(LogTemp, Error,
               TEXT("ComboBoxString_DownloadDBTemplate_Options is nullptr EVSelectImpExpDBOptionsWidget.cpp"));
        return FEVFileOperationInfo();
    }
}