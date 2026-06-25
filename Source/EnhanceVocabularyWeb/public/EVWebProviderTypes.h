#pragma once

#include "CoreMinimal.h"
#include "EVWebProviderTypes.generated.h"

UENUM(BlueprintType)
enum class EEVWebProvider : uint8
{
    FreeDictionary UMETA(DisplayName = "Free Dictionary"),
    MyMemory UMETA(DisplayName = "MyMemory Translation"),
    Datamuse UMETA(DisplayName = "Datamuse")
};

UENUM(BlueprintType)
enum class EEVWebProviderType : uint8
{
    Dictionary,
    Translation,
    Metadata
};

UENUM(BlueprintType)
enum class EEVWebRequestVerb : uint8
{
    GET,
    POST
};

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYWEB_API FEVWebProviderInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EEVWebProvider Provider = EEVWebProvider::FreeDictionary;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString BaseUrl;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EEVWebProviderType ProviderType = EEVWebProviderType::Dictionary;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EEVWebRequestVerb RequestVerb = EEVWebRequestVerb::GET;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bEnabled = true;
};