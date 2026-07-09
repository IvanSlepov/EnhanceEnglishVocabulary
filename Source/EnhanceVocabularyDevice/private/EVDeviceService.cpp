#include "EVDeviceService.h"
#include "EVPlatformFileExchangeServiceFactory.h"

void UEVDeviceService::InitializeDeviceService()
{
    PlatformFileExchangeServiceObject = UEVPlatformFileExchangeServiceFactory::Create(this);

    IEVPlatformFileExchangeService* PlatformService =
        Cast<IEVPlatformFileExchangeService>(PlatformFileExchangeServiceObject);

    if (!PlatformService)
    {
        UE_LOG(LogTemp, Error, TEXT("PlatformFileExchangeService is null or does not implement interface"));
        return;
    }

    PlatformService->OnImportFilePicked().AddUObject(this, &ThisClass::HandlePlatformImportFilePicked);

    PlatformService->OnFileSaved().AddUObject(this, &ThisClass::HandlePlatformFileSaved);
}

void UEVDeviceService::PickImportFile(EEVFileExtensionType FileExtensionType)
{
    IEVPlatformFileExchangeService* PlatformService =
        Cast<IEVPlatformFileExchangeService>(PlatformFileExchangeServiceObject);

    if (!PlatformService)
    {
        return;
    }

    PlatformService->PickImportFile(FileExtensionType);
}

void UEVDeviceService::SaveBytesToUserSelectedLocation(EEVFileExtensionType FileExtensionType,
                                                       const FString& SuggestedFileName, const TArray<uint8>& Bytes)
{
    IEVPlatformFileExchangeService* PlatformService =
        Cast<IEVPlatformFileExchangeService>(PlatformFileExchangeServiceObject);

    if (!PlatformService)
    {
        return;
    }

    PlatformService->SaveBytesToUserSelectedLocation(FileExtensionType, SuggestedFileName, Bytes);
}

FEVDeviceImportFilePicked& UEVDeviceService::OnImportFilePicked()
{
    return ImportFilePickedDelegate;
}

FEVDeviceFileSaved& UEVDeviceService::OnFileSaved()
{
    return FileSavedDelegate;
}

void UEVDeviceService::HandlePlatformImportFilePicked(const FEVFileExchangeResultInfo& ResultInfo,
                                                      const TArray<uint8>& Bytes)
{
    ImportFilePickedDelegate.Broadcast(ResultInfo, Bytes);
}

void UEVDeviceService::HandlePlatformFileSaved(const FEVFileExchangeResultInfo& ResultInfo)
{
    FileSavedDelegate.Broadcast(ResultInfo);
}