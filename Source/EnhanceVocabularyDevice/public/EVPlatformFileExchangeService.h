#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVFileExchangeTypes.h"
#include "EVPlatformFileExchangeService.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FEVOnImportFilePicked, const FEVFileExchangeResultInfo&, const TArray<uint8>&);

DECLARE_MULTICAST_DELEGATE_OneParam(FEVOnFileSaved, const FEVFileExchangeResultInfo&);

UINTERFACE(BlueprintType)
class ENHANCEVOCABULARYDEVICE_API UEVPlatformFileExchangeService : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYDEVICE_API IEVPlatformFileExchangeService
{
    GENERATED_BODY()

public:
    // Optional func
    virtual FString GetPlatformName() const
    {
        return TEXT("Unknown");
    }

    virtual bool SupportsFileExtension(EEVFileExtensionType FileExtensionType) const = 0;

    virtual void PickImportFile(EEVFileExtensionType FileExtensionType) = 0;

    virtual void SaveBytesToUserSelectedLocation(EEVFileExtensionType FileExtensionType,
                                                 const FString& SuggestedFileName, const TArray<uint8>& Bytes) = 0;

    virtual FEVOnImportFilePicked& OnImportFilePicked() = 0;

    virtual FEVOnFileSaved& OnFileSaved() = 0;
};