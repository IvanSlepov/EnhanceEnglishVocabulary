#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVPlatformFileExchangeService.h"
#include "EVDeviceService.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FEVDeviceImportFilePicked, const FEVFileExchangeResultInfo&, const TArray<uint8>&);

DECLARE_MULTICAST_DELEGATE_OneParam(FEVDeviceFileSaved, const FEVFileExchangeResultInfo&);

UCLASS()
class ENHANCEVOCABULARYDEVICE_API UEVDeviceService : public UObject
{
    GENERATED_BODY()

public:
    void InitializeDeviceService();

    void PickImportFile(EEVFileExtensionType FileExtensionType);

    void SaveBytesToUserSelectedLocation(EEVFileExtensionType FileExtensionType, const FString& SuggestedFileName,
                                         const TArray<uint8>& Bytes);

    FEVDeviceImportFilePicked& OnImportFilePicked();
    FEVDeviceFileSaved& OnFileSaved();

private:
    UPROPERTY()
    TObjectPtr<UObject> PlatformFileExchangeServiceObject;

    void HandlePlatformImportFilePicked(const FEVFileExchangeResultInfo& ResultInfo, const TArray<uint8>& Bytes);

    void HandlePlatformFileSaved(const FEVFileExchangeResultInfo& ResultInfo);

    FEVDeviceImportFilePicked ImportFilePickedDelegate;
    FEVDeviceFileSaved FileSavedDelegate;
};