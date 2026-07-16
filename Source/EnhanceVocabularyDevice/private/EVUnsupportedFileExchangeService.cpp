#include "EVUnsupportedFileExchangeService.h"

bool UEVUnsupportedFileExchangeService::SupportsFileExtension(EEVFileExtensionType FileExtensionType) const
{
    return false;
}

void UEVUnsupportedFileExchangeService::PickImportFile(EEVFileExtensionType FileExtensionType)
{
    FEVFileExchangeResultInfo ResultInfo;
    ResultInfo.Result = EEVFileExchangeResult::UnsupportedPlatform;
    ResultInfo.UserMessage = TEXT("File import is not supported on this platform.");
    ResultInfo.DebugMessage = TEXT("UEVUnsupportedFileExchangeService::PickImportFile called.");

    TArray<uint8> EmptyBytes;
    ImportFilePickedDelegate.Broadcast(ResultInfo, EmptyBytes);
}

void UEVUnsupportedFileExchangeService::SaveBytesToUserSelectedLocation(EEVFileExtensionType FileExtensionType,
                                                                        const FString& SuggestedFileName,
                                                                        const TArray<uint8>& Bytes)
{
    FEVFileExchangeResultInfo ResultInfo;
    ResultInfo.Result = EEVFileExchangeResult::UnsupportedPlatform;
    ResultInfo.UserMessage = TEXT("File export is not supported on this platform.");
    ResultInfo.DebugMessage = TEXT("UEVUnsupportedFileExchangeService::SaveBytesToUserSelectedLocation called.");
    ResultInfo.FileName = SuggestedFileName;
    ResultInfo.ByteSize = Bytes.Num();

    FileSavedDelegate.Broadcast(ResultInfo);
}

FEVOnImportFilePicked& UEVUnsupportedFileExchangeService::OnImportFilePicked()
{
    return ImportFilePickedDelegate;
}

FEVOnFileSaved& UEVUnsupportedFileExchangeService::OnFileSaved()
{
    return FileSavedDelegate;
}

void UEVUnsupportedFileExchangeService::LoadBytesFromUserSelectedLocation(EEVFileExtensionType FileExtensionType) {}
