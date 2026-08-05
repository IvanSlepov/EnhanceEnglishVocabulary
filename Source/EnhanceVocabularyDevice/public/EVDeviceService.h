#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVPlatformFileExchangeService.h"
#include "EVDeviceService.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FEVDeviceImportFilePicked, const FEVFileExchangeResultInfo&, const TArray<uint8>&);

DECLARE_MULTICAST_DELEGATE_OneParam(FEVDeviceFileSaved, const FEVFileExchangeResultInfo&);

DECLARE_MULTICAST_DELEGATE(FOnPopUpTimerExpired);

DECLARE_MULTICAST_DELEGATE(FOnVocabularyPopUpClosed);

// In NOT DYNAMIC multicast delegate we don't have to separate types of vars from their names with commas
DECLARE_MULTICAST_DELEGATE_OneParam(FEVNotificationPermissionResult, bool bGranted);

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

    void OpenNotificationSettings();

    bool ShowVocabularyNotification(const FString& Word, const FString& NormalizedWord, const FString& Transcription,
                                    const FString& PartOfSpeech, int32 MeaningDisplayOrder,
                                    const FString& DefinitionText, int32 DefinitionDisplayOrder,
                                    int32 NotificationMode);

    bool ScheduleVocabularyNotifications(int32 IntervalSeconds, const FString& SerializedWords, int32 NotificationMode);
    bool GetStoredVocabularyNotificationSettings(bool& bOutEnabled, int32& OutIntervalSeconds,
                                                 int32& OutNotificationMode) const;
    bool ConsumePendingNotificationWord(FString& OutWord) const;
    bool CancelVocabularyNotifications();

    void HandlePopUpTimerExpired();

    bool AreNotificationsEnabled() const;

    bool RequestNotificationPermission();

    bool HasRequestedNotificationPermission() const;

    FEVNotificationPermissionResult& OnNotificationPermissionResult();

    static void HandleAndroidNotificationPermissionResult(bool bGranted);

    void HandleNotificationPermissionResult(bool bIsGranted);

    void TestAlarm();

protected:
    virtual void BeginDestroy() override;

private:
    UPROPERTY()
    TObjectPtr<UObject> PlatformFileExchangeServiceObject;

    static TWeakObjectPtr<UEVDeviceService> ActiveInstance;

    void HandlePlatformImportFilePicked(const FEVFileExchangeResultInfo& ResultInfo, const TArray<uint8>& Bytes);

    void HandlePlatformFileSaved(const FEVFileExchangeResultInfo& ResultInfo);

    FEVDeviceImportFilePicked ImportFilePickedDelegate;
    FEVDeviceFileSaved FileSavedDelegate;

    FTimerHandle PopUpTimerHandle;

    FEVNotificationPermissionResult NotificationPermissionResultDelegate;
};