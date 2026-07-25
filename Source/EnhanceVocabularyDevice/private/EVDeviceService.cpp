#include "EVDeviceService.h"
#include "EVPlatformFileExchangeServiceFactory.h"

#include "Engine/World.h"
#include "TimerManager.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#endif

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

#if PLATFORM_ANDROID
    if (!RequestNotificationPermission())
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to request notification permission."));
    }
#endif

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

void UEVDeviceService::HandlePopUpTimerExpired()
{
    UE_LOG(LogTemp, Log, TEXT("Pop-up timer expired."));

    OnPopUpTimerExpired.Broadcast();
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