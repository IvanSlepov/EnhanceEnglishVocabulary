#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "UObject/Object.h"
#include "EVHttpService.generated.h"

DECLARE_DELEGATE_ThreeParams(FEVHttpResponseDelegate, bool /*bSuccess*/, int32 /*ResponseCode*/,
                             const FString& /*ResponseBody*/
);

UCLASS()
class ENHANCEVOCABULARYWEB_API UEVHttpService : public UObject
{
    GENERATED_BODY()

public:
    void SendGetRequest(const FString& Url, FEVHttpResponseDelegate Callback, float TimeoutSeconds = 10.0f);

private:
    void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful,
                        FEVHttpResponseDelegate Callback);
};