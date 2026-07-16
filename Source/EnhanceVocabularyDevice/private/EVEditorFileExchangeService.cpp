#include "EVEditorFileExchangeService.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if WITH_EDITOR
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#endif

FString UEVEditorFileExchangeService::GetPlatformName() const
{
#if WITH_EDITOR
    return TEXT("Editor");
#else
    return TEXT("Windows");
#endif
}

bool UEVEditorFileExchangeService::SupportsFileExtension(EEVFileExtensionType FileExtensionType) const
{
    return FileExtensionType == EEVFileExtensionType::Csv;
}

void UEVEditorFileExchangeService::PickImportFile(EEVFileExtensionType FileExtensionType)
{
    FEVFileExchangeResultInfo ResultInfo;
    ResultInfo.Result = EEVFileExchangeResult::UnknownError;
    ResultInfo.UserMessage = TEXT("Editor import picker is not implemented yet.");
    ResultInfo.DebugMessage = TEXT("UEVEditorFileExchangeService::PickImportFile placeholder.");
    ResultInfo.ByteSize = 0;

    TArray<uint8> EmptyBytes;
    ImportFilePickedDelegate.Broadcast(ResultInfo, EmptyBytes);
}

void UEVEditorFileExchangeService::SaveBytesToUserSelectedLocation(EEVFileExtensionType FileExtensionType,
                                                                   const FString& SuggestedFileName,
                                                                   const TArray<uint8>& Bytes)
{
    FEVFileExchangeResultInfo ResultInfo;
    ResultInfo.FileName = SuggestedFileName;
    ResultInfo.ByteSize = Bytes.Num();

    if (!SupportsFileExtension(FileExtensionType))
    {
        ResultInfo.Result = EEVFileExchangeResult::InvalidExtension;
        ResultInfo.UserMessage = TEXT("Selected file type is not supported in Editor.");
        ResultInfo.DebugMessage =
            TEXT("UEVEditorFileExchangeService::SaveBytesToUserSelectedLocation invalid extension.");
        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    if (Bytes.Num() <= 0)
    {
        ResultInfo.Result = EEVFileExchangeResult::EmptyFile;
        ResultInfo.UserMessage = TEXT("Cannot save an empty file.");
        ResultInfo.DebugMessage = TEXT("Editor save received empty byte buffer.");
        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    const FString ExportDirectory = FPaths::ProjectSavedDir() / TEXT("EnhanceVocabulary/Export");

    IFileManager::Get().MakeDirectory(*ExportDirectory, true);

    const FString TargetFilePath = ExportDirectory / SuggestedFileName;

    const bool bSaved = FFileHelper::SaveArrayToFile(Bytes, *TargetFilePath);

    ResultInfo.FileName = TargetFilePath;

    if (!bSaved)
    {
        ResultInfo.Result = EEVFileExchangeResult::WriteFailed;
        ResultInfo.UserMessage = TEXT("Failed to save file.");
        ResultInfo.DebugMessage = FString::Printf(TEXT("Failed to save file to path: %s"), *TargetFilePath);
        FileSavedDelegate.Broadcast(ResultInfo);
        return;
    }

    ResultInfo.Result = EEVFileExchangeResult::Success;
    ResultInfo.UserMessage = TEXT("File saved successfully.");
    ResultInfo.DebugMessage = FString::Printf(TEXT("File saved to path: %s"), *TargetFilePath);

    FileSavedDelegate.Broadcast(ResultInfo);
}

FEVOnImportFilePicked& UEVEditorFileExchangeService::OnImportFilePicked()
{
    return ImportFilePickedDelegate;
}

FEVOnFileSaved& UEVEditorFileExchangeService::OnFileSaved()
{
    return FileSavedDelegate;
}

void UEVEditorFileExchangeService::LoadBytesFromUserSelectedLocation(EEVFileExtensionType FileExtensionType)
{
    FEVFileExchangeResultInfo ResultInfo;

    if (!SupportsFileExtension(FileExtensionType))
    {
        ResultInfo.Result = EEVFileExchangeResult::InvalidExtension;
        ResultInfo.UserMessage = TEXT("Selected file type is not supported in Editor.");
        ResultInfo.DebugMessage = TEXT("Editor load received unsupported extension.");

        ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());
        return;
    }

#if WITH_EDITOR

    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

    if (!DesktopPlatform)
    {
        ResultInfo.Result = EEVFileExchangeResult::ReadFailed;
        ResultInfo.UserMessage = TEXT("Unable to open the file picker.");
        ResultInfo.DebugMessage = TEXT("FDesktopPlatformModule::Get returned nullptr.");

        ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());
        return;
    }

    void* ParentWindowHandle = nullptr;

    if (FSlateApplication::IsInitialized())
    {
        const TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindBestParentWindowForDialogs(nullptr);

        if (ParentWindow.IsValid() && ParentWindow->GetNativeWindow().IsValid())
        {
            ParentWindowHandle = ParentWindow->GetNativeWindow()->GetOSWindowHandle();
        }
    }

    FString FileTypes;

    switch (FileExtensionType)
    {
    case EEVFileExtensionType::Csv:
        FileTypes = TEXT("CSV Files (*.csv)|*.csv");
        break;

    default:
        ResultInfo.Result = EEVFileExchangeResult::InvalidExtension;
        ResultInfo.UserMessage = TEXT("Unsupported import file type.");
        ResultInfo.DebugMessage = TEXT("No Editor file dialog filter configured.");

        ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());

        return;
    }

    TArray<FString> SelectedFiles;

    const bool bFileSelected =
        DesktopPlatform->OpenFileDialog(ParentWindowHandle, TEXT("Select database import file"), FString(), FString(),
                                        FileTypes, EFileDialogFlags::None, SelectedFiles);

    if (!bFileSelected || SelectedFiles.IsEmpty())
    {
        ResultInfo.Result = EEVFileExchangeResult::CancelledByUser;
        ResultInfo.UserMessage = TEXT("File import was cancelled.");
        ResultInfo.DebugMessage = TEXT("Editor file picker returned no selected file.");

        ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());

        return;
    }

    const FString& SelectedFilePath = SelectedFiles[0];

    TArray<uint8> FileBytes;

    if (!FFileHelper::LoadFileToArray(FileBytes, *SelectedFilePath))
    {
        ResultInfo.Result = EEVFileExchangeResult::ReadFailed;
        ResultInfo.UserMessage = TEXT("The selected file could not be read.");
        ResultInfo.DebugMessage = FString::Printf(TEXT("LoadFileToArray failed for: %s"), *SelectedFilePath);

        ResultInfo.FileName = FPaths::GetCleanFilename(SelectedFilePath);

        ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());

        return;
    }

    if (FileBytes.IsEmpty())
    {
        ResultInfo.Result = EEVFileExchangeResult::EmptyFile;
        ResultInfo.UserMessage = TEXT("The selected file is empty.");
        ResultInfo.DebugMessage =
            FString::Printf(TEXT("Selected import file contains no bytes: %s"), *SelectedFilePath);

        ResultInfo.FileName = FPaths::GetCleanFilename(SelectedFilePath);

        ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());

        return;
    }

    ResultInfo.Result = EEVFileExchangeResult::Success;
    ResultInfo.UserMessage = TEXT("Import file loaded successfully.");
    ResultInfo.DebugMessage = FString::Printf(TEXT("Loaded import file from Editor: %s"), *SelectedFilePath);
    ResultInfo.FileName = FPaths::GetCleanFilename(SelectedFilePath);
    ResultInfo.ByteSize = FileBytes.Num();

    ImportFilePickedDelegate.Broadcast(ResultInfo, FileBytes);

#else

    ResultInfo.Result = EEVFileExchangeResult::UnsupportedPlatform;
    ResultInfo.UserMessage = TEXT("Editor file loading is unavailable.");
    ResultInfo.DebugMessage = TEXT("WITH_EDITOR is false.");

    ImportFilePickedDelegate.Broadcast(ResultInfo, TArray<uint8>());

#endif
}