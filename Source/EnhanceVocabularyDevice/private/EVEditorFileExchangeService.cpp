#include "EVEditorFileExchangeService.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

FString UEVEditorFileExchangeService::GetPlatformName() const
{
#if WITH_EDITOR
    return TEXT("Editor");
#else
    return TEXT("Windows");
#endif
}

bool UEVEditorFileExchangeService::SupportsFileExtension(EEVFileExtensionType FileExtensionType) const
{
    return FileExtensionType == EEVFileExtensionType::Csv;
}

void UEVEditorFileExchangeService::PickImportFile(EEVFileExtensionType FileExtensionType)
{
    FEVFileExchangeResultInfo ResultInfo;
    ResultInfo.Result = EEVFileExchangeResult::UnknownError;
    ResultInfo.UserMessage = TEXT("Editor import picker is not implemented yet.");
    ResultInfo.DebugMessage = TEXT("UEVEditorFileExchangeService::PickImportFile placeholder.");
    ResultInfo.ByteSize = 0;

    TArray<uint8> EmptyBytes;
    ImportFilePickedDelegate.Broadcast(ResultInfo, EmptyBytes);
}

void UEVEditorFileExchangeService::SaveBytesToUserSelectedLocation(EEVFileExtensionType FileExtensionType,
                                                                   const FString& SuggestedFileName,
                                                                   const TArray<uint8>& Bytes)
{
    FEVFileExchangeResultInfo ResultInfo;
    ResultInfo.FileName = SuggestedFileName;
    ResultInfo.ByteSize = Bytes.Num();

    if (!SupportsFileExtension(FileExtensionType))
    {
        ResultInfo.Result = EEVFileExchangeResult::InvalidExtension;
        ResultInfo.UserMessage = TEXT("Selected file type is not supported in Editor.");
        ResultInfo.DebugMessage =
            TEXT("UEVEditorFileExchangeService::SaveBytesToUserSelectedLocation invalid extension.");
        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    if (Bytes.Num() <= 0)
    {
        ResultInfo.Result = EEVFileExchangeResult::EmptyFile;
        ResultInfo.UserMessage = TEXT("Cannot save an empty file.");
        ResultInfo.DebugMessage = TEXT("Editor save received empty byte buffer.");
        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    const FString ExportDirectory = FPaths::ProjectSavedDir() / TEXT("EnhanceVocabulary/Export");

    IFileManager::Get().MakeDirectory(*ExportDirectory, true);

    const FString TargetFilePath = ExportDirectory / SuggestedFileName;

    const bool bSaved = FFileHelper::SaveArrayToFile(Bytes, *TargetFilePath);

    ResultInfo.FileName = TargetFilePath;

    if (!bSaved)
    {
        ResultInfo.Result = EEVFileExchangeResult::WriteFailed;
        ResultInfo.UserMessage = TEXT("Failed to save file.");
        ResultInfo.DebugMessage = FString::Printf(TEXT("Failed to save file to path: %s"), *TargetFilePath);
        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    ResultInfo.Result = EEVFileExchangeResult::Success;
    ResultInfo.UserMessage = TEXT("File saved successfully.");
    ResultInfo.DebugMessage = FString::Printf(TEXT("File saved to path: %s"), *TargetFilePath);

    FileSavedDelegate.Broadcast(ResultInfo);
}

FEVOnImportFilePicked& UEVEditorFileExchangeService::OnImportFilePicked()
{
    return ImportFilePickedDelegate;
}

FEVOnFileSaved& UEVEditorFileExchangeService::OnFileSaved()
{
    return FileSavedDelegate;
}