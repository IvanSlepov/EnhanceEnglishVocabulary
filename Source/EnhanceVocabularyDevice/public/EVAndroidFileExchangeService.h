#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVPlatformFileExchangeService.h"
#include "EVAndroidFileExchangeService.generated.h"

UCLASS()
class ENHANCEVOCABULARYDEVICE_API UEVAndroidFileExchangeService final : public UObject,
                                                                        public IEVPlatformFileExchangeService
{
    GENERATED_BODY()

public:
    virtual FString GetPlatformName() const override;

    virtual bool SupportsFileExtension(EEVFileExtensionType FileExtensionType) const override;

    virtual void PickImportFile(EEVFileExtensionType FileExtensionType) override;

    virtual void SaveBytesToUserSelectedLocation(EEVFileExtensionType FileExtensionType,
                                                 const FString& SuggestedFileName, const TArray<uint8>& Bytes) override;

    virtual FEVOnImportFilePicked& OnImportFilePicked() override;

    virtual FEVOnFileSaved& OnFileSaved() override;

    void HandleAndroidFileSaveCompleted(bool bSuccess, bool bCancelled, const FString& FileName,
                                        const FString& ErrorMessage);

    void HandleAndroidImportFilePicked(bool bSuccess, bool bCancelled, const FString& FileName,
                                       const TArray<uint8>& Bytes, const FString& ErrorMessage);

    virtual void LoadBytesFromUserSelectedLocation(EEVFileExtensionType FileExtensionType) override;

private:
    FEVOnImportFilePicked ImportFilePickedDelegate;
    FEVOnFileSaved FileSavedDelegate;

    int64 PendingSaveByteSize = 0;
};