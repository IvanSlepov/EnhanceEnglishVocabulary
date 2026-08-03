#include "EVDeviceService.h"
#include "EVPlatformFileExchangeServiceFactory.h"

#include "Engine/World.h"
#include "TimerManager.h"

#include "Async/Async.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#endif

// Get access to the current GameInstance
TWeakObjectPtr<UEVDeviceService> UEVDeviceService::ActiveInstance;

bool UEVDeviceService::StartPopUpTimer(const int32 IntervalSeconds)
{
    if (IntervalSeconds <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start pop-up timer: invalid interval."));

        return false;
    }

    UWorld* World = GetWorld();

    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot start pop-up timer: world is invalid."));

        return false;
    }

    FTimerManager& TimerManager = World->GetTimerManager();

    TimerManager.ClearTimer(PopUpTimerHandle);

    TimerManager.SetTimer(PopUpTimerHandle, this, &ThisClass::HandlePopUpTimerExpired,
                          static_cast<float>(IntervalSeconds), false);

    UE_LOG(LogTemp, Log, TEXT("Pop-up timer started for %d seconds."), IntervalSeconds);

    return true;
}

bool UEVDeviceService::StopPopUpTimer()
{
    UWorld* World = GetWorld();

    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot stop pop-up timer: world is invalid."));

        return false;
    }

    World->GetTimerManager().ClearTimer(PopUpTimerHandle);

    UE_LOG(LogTemp, Log, TEXT("Pop-up timer stopped."));

    return true;
}

void UEVDeviceService::OpenNotificationSettings()
{
#if PLATFORM_ANDROID

    JNIEnv* Env = FAndroidApplication::GetJavaEnv();

    if (!Env)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot open notification settings: Java environment is invalid."));
        return;
    }

    static jmethodID OpenNotificationSettingsMethod = FJavaWrapper::FindMethod(
        Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_EV_OpenNotificationSettings", "()V", false);

    if (!OpenNotificationSettingsMethod)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot find Android notification settings method."));
        return;
    }

    Env->CallVoidMethod(FJavaWrapper::GameActivityThis, OpenNotificationSettingsMethod);

    if (Env->ExceptionCheck())
    {
        Env->ExceptionDescribe();
        Env->ExceptionClear();

        UE_LOG(LogTemp, Error, TEXT("Java exception occurred while opening notification settings."));
    }

#endif
}

bool UEVDeviceService::ShowVocabularyNotification(const FString& Word)
{
    if (Word.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot show notification: word is empty."));

        return false;
    }

#if PLATFORM_ANDROID

    JNIEnv* Env = FAndroidApplication::GetJavaEnv();

    if (!Env)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot show notification: Java environment is invalid."));

        return false;
    }

    static jmethodID ShowNotificationMethod =
        FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID,
                                 "AndroidThunkJava_EV_ShowVocabularyNotification", "(Ljava/lang/String;)V", false);

    if (!ShowNotificationMethod)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot find Android vocabulary notification method."));

        return false;
    }

    jstring JavaWord = Env->NewStringUTF(TCHAR_TO_UTF8(*Word));

    if (!JavaWord)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to convert vocabulary word to Java string."));

        return false;
    }

    Env->CallVoidMethod(FJavaWrapper::GameActivityThis, ShowNotificationMethod, JavaWord);

    Env->DeleteLocalRef(JavaWord);

    if (Env->ExceptionCheck())
    {
        Env->ExceptionDescribe();
        Env->ExceptionClear();

        UE_LOG(LogTemp, Error, TEXT("Java exception occurred while showing vocabulary notification."));

        return false;
    }

    return true;

#else

    UE_LOG(LogTemp, Warning, TEXT("Vocabulary notification test: %s"), *Word);

    return true;

#endif
}

bool UEVDeviceService::ScheduleVocabularyNotifications(const int32 IntervalSeconds, const FString& SerializedWords,
                                                       const int32 NotificationMode)
{
    if (IntervalSeconds <= 0 || SerializedWords.IsEmpty())
    {
        UE_LOG(LogTemp, Warning,
               TEXT("Cannot schedule vocabulary notifications: invalid interval or empty word payload."));
        return false;
    }

#if PLATFORM_ANDROID
    JNIEnv* Env = FAndroidApplication::GetJavaEnv();
    if (!Env)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot schedule vocabulary notifications: Java environment is invalid."));
        return false;
    }

    static jmethodID ScheduleMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID,
                                                               "AndroidThunkJava_EV_ScheduleVocabularyNotifications",
                                                               "(ILjava/lang/String;I)Z", false);

    if (!ScheduleMethod)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot find Android vocabulary notification scheduling method."));
        return false;
    }

    jstring JavaWords = Env->NewStringUTF(TCHAR_TO_UTF8(*SerializedWords));
    if (!JavaWords)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to convert vocabulary notification payload to Java string."));
        return false;
    }

    const jboolean bScheduled = Env->CallBooleanMethod(FJavaWrapper::GameActivityThis, ScheduleMethod, IntervalSeconds,
                                                       JavaWords, NotificationMode);
    Env->DeleteLocalRef(JavaWords);

    if (Env->ExceptionCheck())
    {
        Env->ExceptionDescribe();
        Env->ExceptionClear();
        UE_LOG(LogTemp, Error, TEXT("Java exception occurred while scheduling vocabulary notifications."));
        return false;
    }

    return bScheduled == JNI_TRUE;
#else
    return StartPopUpTimer(IntervalSeconds);
#endif
}

bool UEVDeviceService::GetStoredVocabularyNotificationSettings(bool& bOutEnabled, int32& OutIntervalSeconds,
                                                               int32& OutNotificationMode) const
{
    bOutEnabled = false;
    OutIntervalSeconds = 0;
    OutNotificationMode = 0;

#if PLATFORM_ANDROID
    JNIEnv* Env = FAndroidApplication::GetJavaEnv();
    if (!Env)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot read notification settings: Java environment is invalid."));
        return false;
    }

    static jmethodID IsEnabledMethod =
        FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID,
                                 "AndroidThunkJava_EV_IsVocabularyNotificationScheduleEnabled", "()Z", false);
    static jmethodID GetIntervalMethod =
        FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID,
                                 "AndroidThunkJava_EV_GetVocabularyNotificationIntervalSeconds", "()I", false);
    static jmethodID GetModeMethod = FJavaWrapper::FindMethod(
        Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_EV_GetVocabularyNotificationMode", "()I", false);

    if (!IsEnabledMethod || !GetIntervalMethod || !GetModeMethod)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot find one or more Android notification settings read methods."));
        return false;
    }

    bOutEnabled = Env->CallBooleanMethod(FJavaWrapper::GameActivityThis, IsEnabledMethod) == JNI_TRUE;
    OutIntervalSeconds = Env->CallIntMethod(FJavaWrapper::GameActivityThis, GetIntervalMethod);
    OutNotificationMode = Env->CallIntMethod(FJavaWrapper::GameActivityThis, GetModeMethod);

    if (Env->ExceptionCheck())
    {
        Env->ExceptionDescribe();
        Env->ExceptionClear();
        UE_LOG(LogTemp, Error, TEXT("Java exception occurred while reading notification settings."));
        return false;
    }

    return true;
#else
    return false;
#endif
}

bool UEVDeviceService::ConsumePendingNotificationWord(FString& OutWord) const
{
    OutWord.Empty();

#if PLATFORM_ANDROID
    JNIEnv* Env = FAndroidApplication::GetJavaEnv();
    if (!Env)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot consume notification word: Java environment is invalid."));
        return false;
    }

    static jmethodID ConsumeMethod =
        FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID,
                                 "AndroidThunkJava_EV_ConsumePendingNotificationWord", "()Ljava/lang/String;", false);

    if (!ConsumeMethod)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot find Android notification word consume method."));
        return false;
    }

    jstring JavaWord = static_cast<jstring>(Env->CallObjectMethod(FJavaWrapper::GameActivityThis, ConsumeMethod));

    if (Env->ExceptionCheck())
    {
        Env->ExceptionDescribe();
        Env->ExceptionClear();
        UE_LOG(LogTemp, Error, TEXT("Java exception occurred while consuming notification word."));
        return false;
    }

    if (JavaWord)
    {
        const char* UtfWord = Env->GetStringUTFChars(JavaWord, nullptr);
        if (UtfWord)
        {
            OutWord = UTF8_TO_TCHAR(UtfWord);
            Env->ReleaseStringUTFChars(JavaWord, UtfWord);
        }
        Env->DeleteLocalRef(JavaWord);
    }

    OutWord.TrimStartAndEndInline();
    return true;
#else
    return true;
#endif
}

bool UEVDeviceService::CancelVocabularyNotifications()
{
#if PLATFORM_ANDROID
    JNIEnv* Env = FAndroidApplication::GetJavaEnv();
    if (!Env)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot cancel vocabulary notifications: Java environment is invalid."));
        return false;
    }

    static jmethodID CancelMethod = FJavaWrapper::FindMethod(
        Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_EV_CancelVocabularyNotifications", "()Z", false);

    if (!CancelMethod)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot find Android vocabulary notification cancellation method."));
        return false;
    }

    const jboolean bCancelled = Env->CallBooleanMethod(FJavaWrapper::GameActivityThis, CancelMethod);

    if (Env->ExceptionCheck())
    {
        Env->ExceptionDescribe();
        Env->ExceptionClear();
        UE_LOG(LogTemp, Error, TEXT("Java exception occurred while cancelling vocabulary notifications."));
        return false;
    }

    return bCancelled == JNI_TRUE;
#else
    return StopPopUpTimer();
#endif
}

void UEVDeviceService::HandlePopUpTimerExpired()
{
    UE_LOG(LogTemp, Log, TEXT("Pop-up timer expired."));

    OnPopUpTimerExpired.Broadcast();
}

bool UEVDeviceService::AreNotificationsEnabled() const
{
#if PLATFORM_ANDROID

    JNIEnv* Env = FAndroidApplication::GetJavaEnv();

    if (!Env)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot check notifications: Java environment is invalid."));

        return false;
    }

    static jmethodID AreNotificationsEnabledMethod = FJavaWrapper::FindMethod(
        Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_EV_AreNotificationsEnabled", "()Z", false);

    if (!AreNotificationsEnabledMethod)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot find Android notification availability method."));

        return false;
    }

    const jboolean bNotificationsEnabled =
        Env->CallBooleanMethod(FJavaWrapper::GameActivityThis, AreNotificationsEnabledMethod);

    if (Env->ExceptionCheck())
    {
        Env->ExceptionDescribe();
        Env->ExceptionClear();

        UE_LOG(LogTemp, Error, TEXT("Java exception occurred while checking notifications."));

        return false;
    }

    return bNotificationsEnabled == JNI_TRUE;

#else

    return true;

#endif
}

bool UEVDeviceService::RequestNotificationPermission()
{
#if PLATFORM_ANDROID

    JNIEnv* Env = FAndroidApplication::GetJavaEnv();

    if (!Env)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot request notification permission: Java environment is invalid."));

        return false;
    }

    static jmethodID RequestPermissionMethod = FJavaWrapper::FindMethod(
        Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_EV_RequestNotificationPermission", "()V", false);

    if (!RequestPermissionMethod)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot find Android notification permission method."));

        return false;
    }

    Env->CallVoidMethod(FJavaWrapper::GameActivityThis, RequestPermissionMethod);

    if (Env->ExceptionCheck())
    {
        Env->ExceptionDescribe();
        Env->ExceptionClear();

        UE_LOG(LogTemp, Error, TEXT("Java exception occurred while requesting notification permission."));

        return false;
    }

    return true;

#else

    return true;

#endif
}

bool UEVDeviceService::HasRequestedNotificationPermission() const
{
#if PLATFORM_ANDROID

    JNIEnv* Env = FAndroidApplication::GetJavaEnv();

    if (!Env)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot check notification permission history: Java environment is invalid."));

        return false;
    }

    static jmethodID HasRequestedPermissionMethod = FJavaWrapper::FindMethod(
        Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_EV_HasRequestedNotificationPermission", "()Z", false);

    if (!HasRequestedPermissionMethod)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot find Android notification permission history method."));

        return false;
    }

    const jboolean bHasRequested = Env->CallBooleanMethod(FJavaWrapper::GameActivityThis, HasRequestedPermissionMethod);

    if (Env->ExceptionCheck())
    {
        Env->ExceptionDescribe();
        Env->ExceptionClear();

        UE_LOG(LogTemp, Error, TEXT("Java exception occurred while checking notification permission history."));

        return false;
    }

    return bHasRequested == JNI_TRUE;

#else

    return true;

#endif
}

FEVNotificationPermissionResult& UEVDeviceService::OnNotificationPermissionResult()
{
    return NotificationPermissionResultDelegate;
}

void UEVDeviceService::HandleAndroidNotificationPermissionResult(const bool bGranted)
{
    AsyncTask(ENamedThreads::GameThread,
              [bGranted]()
              {
                  UEVDeviceService* DeviceService = ActiveInstance.Get();

                  if (!IsValid(DeviceService))
                  {
                      UE_LOG(LogTemp, Warning,
                             TEXT("Notification permission result received, but no active DeviceService exists."));

                      return;
                  }

                  DeviceService->HandleNotificationPermissionResult(bGranted);
              });
}

void UEVDeviceService::HandleNotificationPermissionResult(const bool bIsGranted)
{
    UE_LOG(LogTemp, Log, TEXT("Notification permission result received: %s"),
           bIsGranted ? TEXT("Granted") : TEXT("Denied"));

    NotificationPermissionResultDelegate.Broadcast(bIsGranted);
}

void UEVDeviceService::TestAlarm()
{
#if PLATFORM_ANDROID

    JNIEnv* Env = FAndroidApplication::GetJavaEnv();

    if (!Env)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot schedule test alarm: Java environment is invalid."));
        return;
    }

    static jmethodID TestAlarmMethod =
        FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_EV_TestAlarm", "()V", false);

    if (!TestAlarmMethod)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot find Android test alarm method."));
        return;
    }

    Env->CallVoidMethod(FJavaWrapper::GameActivityThis, TestAlarmMethod);

    if (Env->ExceptionCheck())
    {
        Env->ExceptionDescribe();
        Env->ExceptionClear();

        UE_LOG(LogTemp, Error, TEXT("Java exception occurred while scheduling the test alarm."));
    }

#endif
}

void UEVDeviceService::InitializeDeviceService()
{
    ActiveInstance = this;

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

void UEVDeviceService::BeginDestroy()
{
    if (ActiveInstance.Get() == this)
    {
        ActiveInstance.Reset();
    }

    Super::BeginDestroy();
}

void UEVDeviceService::PickImportFile(EEVFileExtensionType FileExtensionType)
{
    if (!PlatformFileExchangeServiceObject)
    {
        FEVFileExchangeResultInfo ResultInfo;
        ResultInfo.Result = EEVFileExchangeResult::UnsupportedPlatform;
        ResultInfo.UserMessage = TEXT("File import is not available on this platform.");
        ResultInfo.DebugMessage = TEXT("PlatformFileExchangeServiceObject is null in PickImportFile.");

        ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());

        return;
    }

    IEVPlatformFileExchangeService* PlatformFileExchangeService =
        Cast<IEVPlatformFileExchangeService>(PlatformFileExchangeServiceObject);

    if (!PlatformFileExchangeService)
    {
        FEVFileExchangeResultInfo ResultInfo;
        ResultInfo.Result = EEVFileExchangeResult::UnsupportedPlatform;
        ResultInfo.UserMessage = TEXT("File import service is unavailable.");
        ResultInfo.DebugMessage =
            TEXT("PlatformFileExchangeServiceObject does not implement IEVPlatformFileExchangeService.");

        ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());

        return;
    }

    PlatformFileExchangeService->LoadBytesFromUserSelectedLocation(FileExtensionType);
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

#if PLATFORM_ANDROID

extern "C" JNIEXPORT void JNICALL Java_com_epicgames_unreal_GameActivity_nativeEVNotificationPermissionResult(
    JNIEnv* Env, jobject Thiz, jboolean bGranted)
{
    UEVDeviceService::HandleAndroidNotificationPermissionResult(bGranted == JNI_TRUE);
}

#endif