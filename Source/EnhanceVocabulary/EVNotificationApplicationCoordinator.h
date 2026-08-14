#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVPopUpSettingsTypes.h"
#include "EVNotificationApplicationCoordinator.generated.h"

class UEVDeviceService;
class UEVVocabularyStorageService;

DECLARE_MULTICAST_DELEGATE_OneParam(FEVNotificationPermissionResultFromGameInstance, bool /* bGranted */);

UCLASS()
class ENHANCEVOCABULARY_API UEVNotificationApplicationCoordinator : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UEVVocabularyStorageService* InStorageService, UEVDeviceService* InDeviceService);
    void Shutdown();
    bool ApplySettings(const FEVPopUpSettingsInfo& Settings);
    bool GetStoredSettings(FEVPopUpSettingsInfo& OutSettings) const;
    bool ConsumePendingNotificationWord(FString& OutWord) const;
    bool AreNotificationsEnabled() const;
    bool HasRequestedNotificationPermission() const;
    bool RequestNotificationPermission();
    void OpenNotificationSettings();
    void TestDeviceAlarm();
    FEVNotificationPermissionResultFromGameInstance& OnNotificationPermissionResult()
    {
        return NotificationPermissionResultDelegate;
    }

private:
    void HandlePopUpTimerExpired();
    void HandleNotificationPermissionResult(bool bGranted);

    UPROPERTY(Transient)
    TObjectPtr<UEVVocabularyStorageService> VocabularyStorageService;

    UPROPERTY(Transient)
    TObjectPtr<UEVDeviceService> DeviceService;

    FEVPopUpSettingsInfo CurrentSettings;
    FEVNotificationPermissionResultFromGameInstance NotificationPermissionResultDelegate;
};
