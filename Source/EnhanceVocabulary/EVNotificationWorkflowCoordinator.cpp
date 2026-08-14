#include "EVNotificationWorkflowCoordinator.h"

#include "EVApplicationFeatureTypes.h"
#include "EVFeatureNavigationApplicationPort.h"
#include "EVGameInstance.h"
#include "EVNotificationSettingsApplicationPort.h"
#include "EVWidgetCommonEvents.h"

void UEVNotificationWorkflowCoordinator::Initialize(UEVGameInstance* InGameInstance,
                                                    IEVNotificationSettingsApplicationPort* InSettingsPort,
                                                    IEVFeatureNavigationApplicationPort* InNavigationPort,
                                                    IEVWidgetCommonEvents* InLegacyEvents)
{
    GameInstance = InGameInstance;
    UpdatePresentationPorts(InSettingsPort, InNavigationPort, InLegacyEvents);
}

void UEVNotificationWorkflowCoordinator::UpdatePresentationPorts(IEVNotificationSettingsApplicationPort* InSettingsPort,
                                                                 IEVFeatureNavigationApplicationPort* InNavigationPort,
                                                                 IEVWidgetCommonEvents* InLegacyEvents)
{
    SettingsPort = InSettingsPort;
    NavigationPort = InNavigationPort;
    LegacyEvents = InLegacyEvents;
}

void UEVNotificationWorkflowCoordinator::RequestSettingsChange(const FEVPopUpSettingsInfo& RequestedSettings)
{
    if (bTransitionInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Ignoring notification settings change while another transition is pending."));
        ApplyResolvedSettings(CurrentAcceptedSettings);
        return;
    }

    PendingRequestedSettings = RequestedSettings;
    bHasPendingRequestedSettings = true;
    bTransitionInProgress = true;
    EvaluateSettingsChange();
}

void UEVNotificationWorkflowCoordinator::EvaluateSettingsChange()
{
    if (!bHasPendingRequestedSettings)
    {
        bTransitionInProgress = false;
        return;
    }

    const bool bIntervalChanged = HasIntervalChanged();
    const bool bModeChanged = HasModeChanged();
    if (!bIntervalChanged && !bModeChanged)
    {
        ClearPendingRequest();
        ApplyResolvedSettings(CurrentAcceptedSettings);
        return;
    }

    if (bModeChanged)
    {
        if (PendingRequestedSettings.NotificationMode == EEVNotificationMode::RandomWord ||
            PendingRequestedSettings.NotificationMode == EEVNotificationMode::TestMode)
        {
            RequestModeChangeConfirmation();
            return;
        }

        ensureMsgf(false, TEXT("Unsupported notification mode."));
        RejectPendingModeChange();
        return;
    }

    ProcessSettingsRequest(PendingRequestedSettings);
}

void UEVNotificationWorkflowCoordinator::RequestModeChangeConfirmation()
{
    if (!bHasPendingRequestedSettings)
    {
        ClearPendingRequest();
        return;
    }

    ConfirmationRequested.Broadcast(EEVConfirmationDialogType::ChangeNotificationMode, EEVWordEntryActionType::Unknown);
}

bool UEVNotificationWorkflowCoordinator::ResolveConfirmation(const EEVConfirmationDialogType DialogType,
                                                             const bool bConfirmed)
{
    if (DialogType == EEVConfirmationDialogType::ChangeNotificationMode)
    {
        if (bConfirmed)
        {
            CommitPendingModeChange();
        }
        else
        {
            RejectPendingModeChange();
        }
        return true;
    }

    if (DialogType != EEVConfirmationDialogType::EnableAndroidNotifications)
    {
        return false;
    }

    if (!bConfirmed)
    {
        RejectPendingRequest();
        return true;
    }

    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot open notification settings: GameInstance is unavailable."));
        RejectPendingRequest();
        return true;
    }

    bWaitingForNotificationSettings = true;
    GameInstance->OpenNotificationSettings();
    return true;
}

void UEVNotificationWorkflowCoordinator::CommitPendingModeChange()
{
    if (!bHasPendingRequestedSettings)
    {
        ClearPendingRequest();
        ApplyResolvedSettings(CurrentAcceptedSettings);
        return;
    }

    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot change notification mode: GameInstance is unavailable."));
        RejectPendingModeChange();
        return;
    }

    FEVPopUpSettingsInfo ResolvedSettings = PendingRequestedSettings;
    ResolvedSettings.PopUpIntervals = EEVPopUpIntervals::TurnedOff;
    if (!GameInstance->HandlePopUpIntervalSelected(ResolvedSettings))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to stop the notification schedule while changing mode."));
        RejectPendingModeChange();
        return;
    }

    CurrentAcceptedSettings = ResolvedSettings;
    ClearPendingRequest();
    ApplyResolvedSettings(CurrentAcceptedSettings);

    FEVRequestedActionInfo ActionInfo;
    ActionInfo.Source = EEVRequestedActionSource::PopUpSettings;
    ActionInfo.Type = EEVRequestedActionType::EnableNotifications;
    ActionInfo.Status = EEVRequestedActionStatus::Warning;
    ActionInfo.Message =
        FText::FromString(TEXT("Notification mode changed. Select a notification interval to start the new mode."));
    ActionInfo.GenerateColor();
    StatusRequested.Broadcast(ActionInfo);
}

void UEVNotificationWorkflowCoordinator::RejectPendingModeChange()
{
    ClearPendingRequest();
    ApplyResolvedSettings(CurrentAcceptedSettings);
}

void UEVNotificationWorkflowCoordinator::ProcessSettingsRequest(const FEVPopUpSettingsInfo& RequestedSettings)
{
    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot process notification settings: GameInstance is unavailable."));
        RejectPendingRequest();
        return;
    }

    if (RequestedSettings.PopUpIntervals == EEVPopUpIntervals::TurnedOff)
    {
        if (!GameInstance->HandlePopUpIntervalSelected(RequestedSettings))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to stop the notification schedule."));
            RejectPendingRequest();
            return;
        }

        CurrentAcceptedSettings = RequestedSettings;
        ClearPendingRequest();
        ApplyResolvedSettings(CurrentAcceptedSettings);
        return;
    }

    PendingPermissionSettings = RequestedSettings;
    bHasPendingPermissionSettings = true;

    FEVPopUpSettingsInfo InactiveSettings = RequestedSettings;
    InactiveSettings.PopUpIntervals = EEVPopUpIntervals::TurnedOff;
    ApplyResolvedSettings(InactiveSettings);

    if (GameInstance->AreNotificationsEnabled())
    {
        CommitPendingPermissionSettings();
        return;
    }

    if (!GameInstance->HasRequestedNotificationPermission())
    {
        if (!GameInstance->RequestNotificationPermission())
        {
            RejectPendingRequest();
        }
        return;
    }

    ConfirmationRequested.Broadcast(EEVConfirmationDialogType::EnableAndroidNotifications,
                                    EEVWordEntryActionType::Unknown);
}

void UEVNotificationWorkflowCoordinator::ApplyResolvedSettings(const FEVPopUpSettingsInfo& ResolvedSettings)
{
    if (SettingsPort)
    {
        SettingsPort->ApplyNotificationSettingsState(ResolvedSettings);
    }
    else if (LegacyEvents)
    {
        LegacyEvents->HandleApplyResolvedPopUpSettings(ResolvedSettings);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot present notification settings: no presentation port is attached."));
    }
}

void UEVNotificationWorkflowCoordinator::CommitPendingPermissionSettings()
{
    if (!bHasPendingPermissionSettings)
    {
        return;
    }
    if (!GameInstance)
    {
        RejectPendingRequest();
        return;
    }

    const FEVPopUpSettingsInfo SettingsToCommit = PendingPermissionSettings;
    if (!GameInstance->HandlePopUpIntervalSelected(SettingsToCommit))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to apply the pending notification settings."));
        RejectPendingRequest();
        return;
    }

    CurrentAcceptedSettings = SettingsToCommit;
    ClearPendingRequest();
    ApplyResolvedSettings(CurrentAcceptedSettings);
}

void UEVNotificationWorkflowCoordinator::RejectPendingRequest()
{
    bWaitingForNotificationSettings = false;
    bHasPendingPermissionSettings = false;
    PendingPermissionSettings = FEVPopUpSettingsInfo();

    FEVPopUpSettingsInfo DisabledSettings = CurrentAcceptedSettings;
    if (bHasPendingRequestedSettings)
    {
        DisabledSettings.NotificationMode = PendingRequestedSettings.NotificationMode;
    }
    DisabledSettings.PopUpIntervals = EEVPopUpIntervals::TurnedOff;
    if (GameInstance)
    {
        GameInstance->HandlePopUpIntervalSelected(DisabledSettings);
    }

    CurrentAcceptedSettings = DisabledSettings;
    ClearPendingRequest();
    ApplyResolvedSettings(CurrentAcceptedSettings);
}

void UEVNotificationWorkflowCoordinator::ClearPendingRequest()
{
    bHasPendingRequestedSettings = false;
    bHasPendingPermissionSettings = false;
    bWaitingForNotificationSettings = false;
    bTransitionInProgress = false;
    PendingRequestedSettings = FEVPopUpSettingsInfo();
    PendingPermissionSettings = FEVPopUpSettingsInfo();
}

bool UEVNotificationWorkflowCoordinator::HasIntervalChanged() const
{
    return bHasPendingRequestedSettings &&
           PendingRequestedSettings.PopUpIntervals != CurrentAcceptedSettings.PopUpIntervals;
}

bool UEVNotificationWorkflowCoordinator::HasModeChanged() const
{
    return bHasPendingRequestedSettings &&
           PendingRequestedSettings.NotificationMode != CurrentAcceptedSettings.NotificationMode;
}

void UEVNotificationWorkflowCoordinator::HandlePermissionResult(const bool bGranted)
{
    if (!bHasPendingPermissionSettings)
    {
        UE_LOG(LogTemp, Warning,
               TEXT("Notification permission result received without pending notification settings."));
        return;
    }
    if (bGranted && GameInstance && GameInstance->AreNotificationsEnabled())
    {
        CommitPendingPermissionSettings();
    }
    else
    {
        RejectPendingRequest();
    }
}

void UEVNotificationWorkflowCoordinator::HandleApplicationEnteredForeground()
{
    SynchronizeFromDevice();
    HandlePendingNotificationWord();
    if (!bWaitingForNotificationSettings || !bHasPendingPermissionSettings)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Returned from Android notification settings."));
    bWaitingForNotificationSettings = false;
    if (GameInstance && GameInstance->AreNotificationsEnabled())
    {
        CommitPendingPermissionSettings();
    }
    else
    {
        RejectPendingRequest();
    }
}

void UEVNotificationWorkflowCoordinator::PollDeviceState()
{
    if (!bTransitionInProgress)
    {
        SynchronizeFromDevice();
        HandlePendingNotificationWord();
    }
}

void UEVNotificationWorkflowCoordinator::HandlePendingNotificationWord()
{
    if (!GameInstance || (!NavigationPort && !LegacyEvents) ||
        CurrentAcceptedSettings.NotificationMode != EEVNotificationMode::RandomWord)
    {
        return;
    }

    FString NotificationWord;
    if (!GameInstance->ConsumePendingNotificationWord(NotificationWord) || NotificationWord.IsEmpty())
    {
        return;
    }

    if (NavigationPort)
    {
        FEVFeatureNavigationRequest Request;
        Request.FeatureId = EVApplicationFeature::ReviewWords;
        Request.Context = NotificationWord;
        NavigationPort->ApplyFeatureNavigation(Request);
    }
    else
    {
        LegacyEvents->HandleOpenReviewWordsForNotification(NotificationWord);
    }
}

void UEVNotificationWorkflowCoordinator::SynchronizeFromDevice()
{
    if (!GameInstance || (!SettingsPort && !LegacyEvents))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot synchronize notification settings: coordinator is not initialized."));
        return;
    }

    FEVPopUpSettingsInfo StoredSettings;
    if (!GameInstance->GetStoredNotificationSettings(StoredSettings))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to read stored notification settings from the device."));
        return;
    }
    if (StoredSettings.PopUpIntervals == CurrentAcceptedSettings.PopUpIntervals &&
        StoredSettings.NotificationMode == CurrentAcceptedSettings.NotificationMode)
    {
        return;
    }

    CurrentAcceptedSettings = StoredSettings;
    ApplyResolvedSettings(CurrentAcceptedSettings);
}
