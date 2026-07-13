#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVPlatformFileExchangeService.h"
#include "EVUnsupportedFileExchangeService.generated.h"

UCLASS()
class UEVUnsupportedFileExchangeService final : public UObject, public IEVPlatformFileExchangeService
{
    GENERATED_BODY()

public:
    virtual bool SupportsFileExtension(EEVFileExtensionType FileExtensionType) const override;

    virtual void PickImportFile(EEVFileExtensionType FileExtensionType) override;

    virtual void SaveBytesToUserSelectedLocation(EEVFileExtensionType FileExtensionType,
                                                 const FString& SuggestedFileName, const TArray<uint8>& Bytes) override;

    virtual FEVOnImportFilePicked& OnImportFilePicked() override;

    virtual FEVOnFileSaved& OnFileSaved() override;

    virtual void LoadBytesFromUserSelectedLocation(EEVFileExtensionType FileExtensionType) override;

private:
    FEVOnImportFilePicked ImportFilePickedDelegate;
    FEVOnFileSaved FileSavedDelegate;
};