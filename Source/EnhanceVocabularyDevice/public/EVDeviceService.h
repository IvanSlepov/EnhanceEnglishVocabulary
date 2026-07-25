#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVPlatformFileExchangeService.h"
#include "EVDeviceService.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FEVDeviceImportFilePicked, const FEVFileExchangeResultInfo&, const TArray<uint8>&);

DECLARE_MULTICAST_DELEGATE_OneParam(FEVDeviceFileSaved, const FEVFileExchangeResultInfo&);

DECLARE_MULTICAST_DELEGATE(FOnPopUpTimerExpired);

DECLARE_MULTICAST_DELEGATE(FOnVocabularyPopUpClosed);

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

    FOnPopUpTimerExpired OnPopUpTimerExpired; 
    FOnVocabularyPopUpClosed OnVocabularyPopUpClosed;

    bool StartPopUpTimer(int32 IntervalSeconds);
    bool StopPopUpTimer();

    bool ShowVocabularyNotification(const FString& Word);

private:
    UPROPERTY()
    TObjectPtr<UObject> PlatformFileExchangeServiceObject;

    void HandlePlatformImportFilePicked(const FEVFileExchangeResultInfo& ResultInfo, const TArray<uint8>& Bytes);

    void HandlePlatformFileSaved(const FEVFileExchangeResultInfo& ResultInfo);

    void HandlePopUpTimerExpired();

    bool RequestNotificationPermission();

    FEVDeviceImportFilePicked ImportFilePickedDelegate;
    FEVDeviceFileSaved FileSavedDelegate;


    FTimerHandle PopUpTimerHandle;
};