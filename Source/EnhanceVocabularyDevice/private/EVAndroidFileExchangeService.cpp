#include "EVAndroidFileExchangeService.h"

#include "Async/Async.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#endif

namespace
{
#if PLATFORM_ANDROID
TWeakObjectPtr<UEVAndroidFileExchangeService> GEVAndroidFileExchangeService;
#endif
} // namespace

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
    LoadBytesFromUserSelectedLocation(FileExtensionType);
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
        ResultInfo.DebugMessage = TEXT("Android save received an unsupported extension.");

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
        PendingSaveByteSize = 0;

        ResultInfo.Result = EEVFileExchangeResult::WriteFailed;
        ResultInfo.UserMessage = TEXT("Unable to access Android file services.");
        ResultInfo.DebugMessage = TEXT("FAndroidApplication::GetJavaEnv returned nullptr during save.");

        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    static jmethodID SaveMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID,
                                                           "AndroidThunkJava_EV_SaveBytesToUserSelectedLocation",
                                                           "(Ljava/lang/String;Ljava/lang/String;[B)V", false);

    if (!SaveMethod)
    {
        PendingSaveByteSize = 0;

        ResultInfo.Result = EEVFileExchangeResult::WriteFailed;
        ResultInfo.UserMessage = TEXT("Android file-save bridge is unavailable.");
        ResultInfo.DebugMessage = TEXT("Could not find AndroidThunkJava_EV_SaveBytesToUserSelectedLocation.");

        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    FString MimeType;

    switch (FileExtensionType)
    {
    case EEVFileExtensionType::Csv:
        MimeType = TEXT("text/csv");
        break;

    default:
        PendingSaveByteSize = 0;

        ResultInfo.Result = EEVFileExchangeResult::InvalidExtension;
        ResultInfo.UserMessage = TEXT("Unsupported Android save file type.");
        ResultInfo.DebugMessage = TEXT("No MIME type is configured for the selected save extension.");

        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

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

        PendingSaveByteSize = 0;

        ResultInfo.Result = EEVFileExchangeResult::WriteFailed;
        ResultInfo.UserMessage = TEXT("Unable to prepare the Android file.");
        ResultInfo.DebugMessage = TEXT("Failed to allocate one or more JNI values for save.");

        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    Env->SetByteArrayRegion(JavaBytes, 0, Bytes.Num(), reinterpret_cast<const jbyte*>(Bytes.GetData()));

    if (Env->ExceptionCheck())
    {
        Env->ExceptionDescribe();
        Env->ExceptionClear();

        Env->DeleteLocalRef(JavaFileName);
        Env->DeleteLocalRef(JavaMimeType);
        Env->DeleteLocalRef(JavaBytes);

        PendingSaveByteSize = 0;

        ResultInfo.Result = EEVFileExchangeResult::WriteFailed;
        ResultInfo.UserMessage = TEXT("Unable to prepare the Android file data.");
        ResultInfo.DebugMessage = TEXT("SetByteArrayRegion failed while preparing save bytes.");

        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

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

void UEVAndroidFileExchangeService::LoadBytesFromUserSelectedLocation(EEVFileExtensionType FileExtensionType)
{
    FEVFileExchangeResultInfo ResultInfo;

    if (!SupportsFileExtension(FileExtensionType))
    {
        ResultInfo.Result = EEVFileExchangeResult::InvalidExtension;
        ResultInfo.UserMessage = TEXT("Selected file type is not supported on Android.");
        ResultInfo.DebugMessage = TEXT("Android import received an unsupported extension.");

        ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());

        return;
    }

#if PLATFORM_ANDROID

    GEVAndroidFileExchangeService = this;

    JNIEnv* Env = FAndroidApplication::GetJavaEnv();

    if (!Env)
    {
        ResultInfo.Result = EEVFileExchangeResult::ReadFailed;
        ResultInfo.UserMessage = TEXT("Unable to access Android file services.");
        ResultInfo.DebugMessage = TEXT("FAndroidApplication::GetJavaEnv returned nullptr during import.");

        ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());

        return;
    }

    static jmethodID PickImportFileMethod = FJavaWrapper::FindMethod(
        Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_EV_PickImportFile", "(Ljava/lang/String;)V", false);

    if (!PickImportFileMethod)
    {
        ResultInfo.Result = EEVFileExchangeResult::ReadFailed;
        ResultInfo.UserMessage = TEXT("Android file-import bridge is unavailable.");
        ResultInfo.DebugMessage = TEXT("Could not find AndroidThunkJava_EV_PickImportFile.");

        ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());

        return;
    }

    FString MimeType;

    switch (FileExtensionType)
    {
    case EEVFileExtensionType::Csv:
        MimeType = TEXT("text/csv");
        break;

    default:
        ResultInfo.Result = EEVFileExchangeResult::InvalidExtension;
        ResultInfo.UserMessage = TEXT("Unsupported Android import file type.");
        ResultInfo.DebugMessage = TEXT("No MIME type is configured for the selected import extension.");

        ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());

        return;
    }

    jstring JavaMimeType = Env->NewStringUTF(TCHAR_TO_UTF8(*MimeType));

    if (!JavaMimeType)
    {
        ResultInfo.Result = EEVFileExchangeResult::ReadFailed;
        ResultInfo.UserMessage = TEXT("Unable to prepare the Android file picker.");
        ResultInfo.DebugMessage = TEXT("Failed to allocate the MIME-type JNI string.");

        ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());

        return;
    }

    FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, PickImportFileMethod, JavaMimeType);

    Env->DeleteLocalRef(JavaMimeType);

#else

    ResultInfo.Result = EEVFileExchangeResult::UnsupportedPlatform;
    ResultInfo.UserMessage = TEXT("Android file import was called on a non-Android build.");
    ResultInfo.DebugMessage = TEXT("PLATFORM_ANDROID is false.");

    ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());

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
            ErrorMessage.IsEmpty() ? TEXT("Android SAF save failed without an error message.") : ErrorMessage;

        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    ResultInfo.Result = EEVFileExchangeResult::Success;
    ResultInfo.UserMessage = TEXT("File saved successfully.");
    ResultInfo.DebugMessage = TEXT("Android SAF wrote the file successfully.");

    FileSavedDelegate.Broadcast(ResultInfo);
}

void UEVAndroidFileExchangeService::HandleAndroidImportFilePicked(bool bSuccess, bool bCancelled,
                                                                  const FString& FileName, const TArray<uint8>& Bytes,
                                                                  const FString& ErrorMessage)
{
    FEVFileExchangeResultInfo ResultInfo;
    ResultInfo.FileName = FileName;
    ResultInfo.ByteSize = Bytes.Num();

    if (bCancelled)
    {
        ResultInfo.Result = EEVFileExchangeResult::CancelledByUser;
        ResultInfo.UserMessage = TEXT("File import was cancelled.");
        ResultInfo.DebugMessage = TEXT("The Android document picker was cancelled.");

        ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());

        return;
    }

    if (!bSuccess)
    {
        ResultInfo.Result = EEVFileExchangeResult::ReadFailed;
        ResultInfo.UserMessage = TEXT("The selected import file could not be read.");
        ResultInfo.DebugMessage =
            ErrorMessage.IsEmpty() ? TEXT("Android SAF import failed without an error message.") : ErrorMessage;

        ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());

        return;
    }

    if (Bytes.IsEmpty())
    {
        ResultInfo.Result = EEVFileExchangeResult::EmptyFile;
        ResultInfo.UserMessage = TEXT("The selected import file is empty.");
        ResultInfo.DebugMessage = TEXT("Android import callback succeeded but returned no bytes.");

        ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());

        return;
    }

    ResultInfo.Result = EEVFileExchangeResult::Success;
    ResultInfo.UserMessage = TEXT("Import file loaded successfully.");
    ResultInfo.DebugMessage = TEXT("Android SAF read the selected import file successfully.");

    ImportFilePickedDelegate.Broadcast(ResultInfo, Bytes);
}

#if PLATFORM_ANDROID

extern "C" JNIEXPORT void JNICALL Java_com_epicgames_unreal_GameActivity_nativeEVFileSaveCompleted(
    JNIEnv* Env, jobject ActivityObject, jboolean bSuccess, jboolean bCancelled, jstring JFileName,
    jstring JErrorMessage)
{
    (void)ActivityObject;

    FString FileName;
    FString ErrorMessage;

    if (JFileName)
    {
        const char* FileNameChars = Env->GetStringUTFChars(JFileName, nullptr);

        if (FileNameChars)
        {
            FileName = UTF8_TO_TCHAR(FileNameChars);

            Env->ReleaseStringUTFChars(JFileName, FileNameChars);
        }
    }

    if (JErrorMessage)
    {
        const char* ErrorMessageChars = Env->GetStringUTFChars(JErrorMessage, nullptr);

        if (ErrorMessageChars)
        {
            ErrorMessage = UTF8_TO_TCHAR(ErrorMessageChars);

            Env->ReleaseStringUTFChars(JErrorMessage, ErrorMessageChars);
        }
    }

    const bool bWasSuccessful = bSuccess == JNI_TRUE;

    const bool bWasCancelled = bCancelled == JNI_TRUE;

    AsyncTask(ENamedThreads::GameThread,
              [bWasSuccessful, bWasCancelled, FileName, ErrorMessage]()
              {
                  if (!GEVAndroidFileExchangeService.IsValid())
                  {
                      UE_LOG(LogTemp, Error,
                             TEXT("Android file exchange service is no longer valid during save callback."));

                      return;
                  }

                  GEVAndroidFileExchangeService->HandleAndroidFileSaveCompleted(bWasSuccessful, bWasCancelled, FileName,
                                                                                ErrorMessage);
              });
}

extern "C" JNIEXPORT void JNICALL Java_com_epicgames_unreal_GameActivity_nativeEVImportFilePicked(
    JNIEnv* Env, jobject ActivityObject, jboolean bSuccess, jboolean bCancelled, jstring JFileName, jbyteArray JBytes,
    jstring JErrorMessage)
{
    (void)ActivityObject;

    FString FileName;
    FString ErrorMessage;
    TArray<uint8> Bytes;

    if (JFileName)
    {
        const char* FileNameChars = Env->GetStringUTFChars(JFileName, nullptr);

        if (FileNameChars)
        {
            FileName = UTF8_TO_TCHAR(FileNameChars);

            Env->ReleaseStringUTFChars(JFileName, FileNameChars);
        }
    }

    if (JErrorMessage)
    {
        const char* ErrorMessageChars = Env->GetStringUTFChars(JErrorMessage, nullptr);

        if (ErrorMessageChars)
        {
            ErrorMessage = UTF8_TO_TCHAR(ErrorMessageChars);

            Env->ReleaseStringUTFChars(JErrorMessage, ErrorMessageChars);
        }
    }

    if (JBytes)
    {
        const jsize ByteCount = Env->GetArrayLength(JBytes);

        if (ByteCount > 0)
        {
            Bytes.SetNumUninitialized(ByteCount);

            Env->GetByteArrayRegion(JBytes, 0, ByteCount, reinterpret_cast<jbyte*>(Bytes.GetData()));

            if (Env->ExceptionCheck())
            {
                Env->ExceptionDescribe();
                Env->ExceptionClear();

                Bytes.Reset();

                if (ErrorMessage.IsEmpty())
                {
                    ErrorMessage = TEXT("Failed to copy Android import bytes.");
                }
            }
        }
    }

    const bool bWasSuccessful = bSuccess == JNI_TRUE;

    const bool bWasCancelled = bCancelled == JNI_TRUE;

    AsyncTask(ENamedThreads::GameThread,
              [bWasSuccessful, bWasCancelled, FileName, Bytes = MoveTemp(Bytes), ErrorMessage]() mutable
              {
                  if (!GEVAndroidFileExchangeService.IsValid())
                  {
                      UE_LOG(LogTemp, Error,
                             TEXT("Android file exchange service is no longer valid during import callback."));

                      return;
                  }

                  GEVAndroidFileExchangeService->HandleAndroidImportFilePicked(bWasSuccessful, bWasCancelled, FileName,
                                                                               Bytes, ErrorMessage);
              });
}

#endif