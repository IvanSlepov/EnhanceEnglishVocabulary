#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVApplicationWorkflowSignals.h"
#include "EVPopUpSettingsTypes.h"
#include "EVNotificationWorkflowCoordinator.generated.h"

class UEVGameInstance;
class IEVFeatureNavigationApplicationPort;
class IEVNotificationSettingsApplicationPort;
class IEVWidgetCommonEvents;

UCLASS()
class ENHANCEVOCABULARY_API UEVNotificationWorkflowCoordinator : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UEVGameInstance* InGameInstance, IEVNotificationSettingsApplicationPort* InSettingsPort,
                    IEVFeatureNavigationApplicationPort* InNavigationPort, IEVWidgetCommonEvents* InLegacyEvents);
    void UpdatePresentationPorts(IEVNotificationSettingsApplicationPort* InSettingsPort,
                                 IEVFeatureNavigationApplicationPort* InNavigationPort,
                                 IEVWidgetCommonEvents* InLegacyEvents);
    void RequestSettingsChange(const FEVPopUpSettingsInfo& RequestedSettings);
    bool ResolveConfirmation(EEVConfirmationDialogType DialogType, bool bConfirmed);
    void HandlePermissionResult(bool bGranted);
    void HandleApplicationEnteredForeground();
    void PollDeviceState();
    void SynchronizeFromDevice();
    void HandlePendingNotificationWord();

    FOnEVWorkflowConfirmationRequested& OnConfirmationRequested()
    {
        return ConfirmationRequested;
    }
    FOnEVWorkflowStatusRequested& OnStatusRequested()
    {
        return StatusRequested;
    }

private:
    void EvaluateSettingsChange();
    void RequestModeChangeConfirmation();
    void CommitPendingModeChange();
    void RejectPendingModeChange();
    void ProcessSettingsRequest(const FEVPopUpSettingsInfo& RequestedSettings);
    void ApplyResolvedSettings(const FEVPopUpSettingsInfo& ResolvedSettings);
    void CommitPendingPermissionSettings();
    void RejectPendingRequest();
    void ClearPendingRequest();
    bool HasIntervalChanged() const;
    bool HasModeChanged() const;

    UPROPERTY(Transient)
    TObjectPtr<UEVGameInstance> GameInstance;

    IEVNotificationSettingsApplicationPort* SettingsPort = nullptr;
    IEVFeatureNavigationApplicationPort* NavigationPort = nullptr;
    IEVWidgetCommonEvents* LegacyEvents = nullptr;

    FEVPopUpSettingsInfo CurrentAcceptedSettings;
    FEVPopUpSettingsInfo PendingRequestedSettings;
    FEVPopUpSettingsInfo PendingPermissionSettings;
    bool bHasPendingRequestedSettings = false;
    bool bHasPendingPermissionSettings = false;
    bool bWaitingForNotificationSettings = false;
    bool bTransitionInProgress = false;

    FOnEVWorkflowConfirmationRequested ConfirmationRequested;
    FOnEVWorkflowStatusRequested StatusRequested;
};
