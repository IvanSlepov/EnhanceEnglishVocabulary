#include "EVAndroidFileExchangeService.h"
#include "Async/Async.h"

namespace
{
#if PLATFORM_ANDROID
TWeakObjectPtr<UEVAndroidFileExchangeService> GEVAndroidFileExchangeService;
#endif
} // namespace

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#endif

FString UEVAndroidFileExchangeService::GetPlatformName() const
{
    return TEXT("Android");
}

bool UEVAndroidFileExchangeService::SupportsFileExtension(EEVFileExtensionType FileExtensionType) const
{
    return FileExtensionType == EEVFileExtensionType::Csv;
}

void UEVAndroidFileExchangeService::PickImportFile(EEVFileExtensionType FileExtensionType)
{
    FEVFileExchangeResultInfo ResultInfo;

    if (!SupportsFileExtension(FileExtensionType))
    {
        ResultInfo.Result = EEVFileExchangeResult::InvalidExtension;
        ResultInfo.UserMessage = TEXT("Selected file type is not supported on Android.");
        ResultInfo.DebugMessage = TEXT("UEVAndroidFileExchangeService::PickImportFile invalid extension.");

        TArray<uint8> EmptyBytes;
        ImportFilePickedDelegate.Broadcast(ResultInfo, EmptyBytes);
        return;
    }

    ResultInfo.Result = EEVFileExchangeResult::UnsupportedPlatform;
    ResultInfo.UserMessage = TEXT("Android import is not implemented yet.");
    ResultInfo.DebugMessage = TEXT("Android SAF import bridge is pending.");

    TArray<uint8> EmptyBytes;
    ImportFilePickedDelegate.Broadcast(ResultInfo, EmptyBytes);
}

void UEVAndroidFileExchangeService::SaveBytesToUserSelectedLocation(EEVFileExtensionType FileExtensionType,
                                                                    const FString& SuggestedFileName,
                                                                    const TArray<uint8>& Bytes)
{
    FEVFileExchangeResultInfo ResultInfo;
    ResultInfo.FileName = SuggestedFileName;
    ResultInfo.ByteSize = Bytes.Num();

    if (!SupportsFileExtension(FileExtensionType))
    {
        ResultInfo.Result = EEVFileExchangeResult::InvalidExtension;
        ResultInfo.UserMessage = TEXT("Selected file type is not supported on Android.");
        ResultInfo.DebugMessage = TEXT("Android save received unsupported extension.");

        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    if (Bytes.IsEmpty())
    {
        ResultInfo.Result = EEVFileExchangeResult::EmptyFile;
        ResultInfo.UserMessage = TEXT("Cannot save an empty file.");
        ResultInfo.DebugMessage = TEXT("Android save received an empty byte buffer.");

        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

#if PLATFORM_ANDROID

    GEVAndroidFileExchangeService = this;
    PendingSaveByteSize = Bytes.Num();

    JNIEnv* Env = FAndroidApplication::GetJavaEnv();

    if (!Env)
    {
        ResultInfo.Result = EEVFileExchangeResult::WriteFailed;
        ResultInfo.UserMessage = TEXT("Unable to access Android file services.");
        ResultInfo.DebugMessage = TEXT("FAndroidApplication::GetJavaEnv returned nullptr.");

        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    static jmethodID SaveMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID,
                                                           "AndroidThunkJava_EV_SaveBytesToUserSelectedLocation",
                                                           "(Ljava/lang/String;Ljava/lang/String;[B)V", false);

    if (!SaveMethod)
    {
        ResultInfo.Result = EEVFileExchangeResult::WriteFailed;
        ResultInfo.UserMessage = TEXT("Android file save bridge is unavailable.");
        ResultInfo.DebugMessage = TEXT("Could not find AndroidThunkJava_EV_SaveBytesToUserSelectedLocation.");

        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    const FString MimeType = TEXT("text/csv");

    jstring JavaFileName = Env->NewStringUTF(TCHAR_TO_UTF8(*SuggestedFileName));

    jstring JavaMimeType = Env->NewStringUTF(TCHAR_TO_UTF8(*MimeType));

    jbyteArray JavaBytes = Env->NewByteArray(Bytes.Num());

    if (!JavaFileName || !JavaMimeType || !JavaBytes)
    {
        if (JavaFileName)
        {
            Env->DeleteLocalRef(JavaFileName);
        }

        if (JavaMimeType)
        {
            Env->DeleteLocalRef(JavaMimeType);
        }

        if (JavaBytes)
        {
            Env->DeleteLocalRef(JavaBytes);
        }

        ResultInfo.Result = EEVFileExchangeResult::WriteFailed;
        ResultInfo.UserMessage = TEXT("Unable to prepare the Android file.");
        ResultInfo.DebugMessage = TEXT("Failed to allocate one or more JNI values.");

        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    Env->SetByteArrayRegion(JavaBytes, 0, Bytes.Num(), reinterpret_cast<const jbyte*>(Bytes.GetData()));

    FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, SaveMethod, JavaFileName, JavaMimeType,
                                 JavaBytes);

    Env->DeleteLocalRef(JavaFileName);
    Env->DeleteLocalRef(JavaMimeType);
    Env->DeleteLocalRef(JavaBytes);
#else
    ResultInfo.Result = EEVFileExchangeResult::UnsupportedPlatform;
    ResultInfo.UserMessage = TEXT("Android save was called on a non-Android build.");
    ResultInfo.DebugMessage = TEXT("PLATFORM_ANDROID is false.");

    FileSavedDelegate.Broadcast(ResultInfo);
#endif
}

FEVOnImportFilePicked& UEVAndroidFileExchangeService::OnImportFilePicked()
{
    return ImportFilePickedDelegate;
}

FEVOnFileSaved& UEVAndroidFileExchangeService::OnFileSaved()
{
    return FileSavedDelegate;
}

void UEVAndroidFileExchangeService::HandleAndroidFileSaveCompleted(bool bSuccess, bool bCancelled,
                                                                   const FString& FileName, const FString& ErrorMessage)
{
    FEVFileExchangeResultInfo ResultInfo;
    ResultInfo.FileName = FileName;
    ResultInfo.ByteSize = PendingSaveByteSize;

    PendingSaveByteSize = 0;

    if (bCancelled)
    {
        ResultInfo.Result = EEVFileExchangeResult::CancelledByUser;
        ResultInfo.UserMessage = TEXT("File save was cancelled.");
        ResultInfo.DebugMessage = TEXT("The Android document picker was cancelled.");

        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    if (!bSuccess)
    {
        ResultInfo.Result = EEVFileExchangeResult::WriteFailed;
        ResultInfo.UserMessage = TEXT("Failed to save file.");
        ResultInfo.DebugMessage =
            ErrorMessage.IsEmpty() ? TEXT("Android SAF failed without an error message.") : ErrorMessage;

        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    ResultInfo.Result = EEVFileExchangeResult::Success;
    ResultInfo.UserMessage = TEXT("Template downloaded successfully.");
    ResultInfo.DebugMessage = TEXT("Android SAF wrote the file successfully.");

    FileSavedDelegate.Broadcast(ResultInfo);
}

#if PLATFORM_ANDROID

extern "C" JNIEXPORT void JNICALL Java_com_epicgames_unreal_GameActivity_nativeEVFileSaveCompleted(
    JNIEnv* Env, jobject ActivityObject, jboolean bSuccess, jboolean bCancelled, jstring JFileName,
    jstring JErrorMessage)
{
    FString FileName;
    FString ErrorMessage;

    if (JFileName)
    {
        const char* FileNameChars = Env->GetStringUTFChars(JFileName, nullptr);

        FileName = UTF8_TO_TCHAR(FileNameChars);

        Env->ReleaseStringUTFChars(JFileName, FileNameChars);
    }

    if (JErrorMessage)
    {
        const char* ErrorMessageChars = Env->GetStringUTFChars(JErrorMessage, nullptr);

        ErrorMessage = UTF8_TO_TCHAR(ErrorMessageChars);

        Env->ReleaseStringUTFChars(JErrorMessage, ErrorMessageChars);
    }

    const bool bWasSuccessful = bSuccess == JNI_TRUE;
    const bool bWasCancelled = bCancelled == JNI_TRUE;

    AsyncTask(ENamedThreads::GameThread,
              [bWasSuccessful, bWasCancelled, FileName, ErrorMessage]()
              {
                  if (!GEVAndroidFileExchangeService.IsValid())
                  {
                      UE_LOG(LogTemp, Error, TEXT("Android file exchange service is no longer valid."));
                      return;
                  }

                  GEVAndroidFileExchangeService->HandleAndroidFileSaveCompleted(bWasSuccessful, bWasCancelled, FileName,
                                                                                ErrorMessage);
              });
}

#endif