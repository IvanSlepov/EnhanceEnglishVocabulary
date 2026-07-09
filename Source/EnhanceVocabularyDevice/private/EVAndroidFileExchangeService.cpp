#include "EVAndroidFileExchangeService.h"

FString UEVAndroidFileExchangeService::GetPlatformName() const
{
    return TEXT("Android");
}

bool UEVAndroidFileExchangeService::SupportsFileExtension(EEVFileExtensionType FileExtensionType) const
{
    return FileExtensionType == EEVFileExtensionType::Csv;
}

void UEVAndroidFileExchangeService::PickImportFile(EEVFileExtensionType FileExtensionType)
{
    FEVFileExchangeResultInfo ResultInfo;

    if (!SupportsFileExtension(FileExtensionType))
    {
        ResultInfo.Result = EEVFileExchangeResult::InvalidExtension;
        ResultInfo.UserMessage = TEXT("Selected file type is not supported on Android.");
        ResultInfo.DebugMessage = TEXT("UEVAndroidFileExchangeService::PickImportFile invalid extension.");

        TArray<uint8> EmptyBytes;
        ImportFilePickedDelegate.Broadcast(ResultInfo, EmptyBytes);
        return;
    }

    ResultInfo.Result = EEVFileExchangeResult::UnsupportedPlatform;
    ResultInfo.UserMessage = TEXT("Android import is not implemented yet.");
    ResultInfo.DebugMessage = TEXT("Android SAF import bridge is pending.");

    TArray<uint8> EmptyBytes;
    ImportFilePickedDelegate.Broadcast(ResultInfo, EmptyBytes);
}

void UEVAndroidFileExchangeService::SaveBytesToUserSelectedLocation(EEVFileExtensionType FileExtensionType,
                                                                    const FString& SuggestedFileName,
                                                                    const TArray<uint8>& Bytes)
{
    FEVFileExchangeResultInfo ResultInfo;
    ResultInfo.FileName = SuggestedFileName;
    ResultInfo.ByteSize = Bytes.Num();

    if (!SupportsFileExtension(FileExtensionType))
    {
        ResultInfo.Result = EEVFileExchangeResult::InvalidExtension;
        ResultInfo.UserMessage = TEXT("Selected file type is not supported on Android.");
        ResultInfo.DebugMessage =
            TEXT("UEVAndroidFileExchangeService::SaveBytesToUserSelectedLocation invalid extension.");

        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    if (Bytes.Num() <= 0)
    {
        ResultInfo.Result = EEVFileExchangeResult::EmptyFile;
        ResultInfo.UserMessage = TEXT("Cannot save an empty file.");
        ResultInfo.DebugMessage = TEXT("Android save received empty byte buffer.");

        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    ResultInfo.Result = EEVFileExchangeResult::UnsupportedPlatform;
    ResultInfo.UserMessage = TEXT("Android export is not implemented yet.");
    ResultInfo.DebugMessage = TEXT("Android SAF save bridge is pending.");

    FileSavedDelegate.Broadcast(ResultInfo);
}

FEVOnImportFilePicked& UEVAndroidFileExchangeService::OnImportFilePicked()
{
    return ImportFilePickedDelegate;
}

FEVOnFileSaved& UEVAndroidFileExchangeService::OnFileSaved()
{
    return FileSavedDelegate;
}