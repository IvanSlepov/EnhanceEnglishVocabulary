#include "EVHttpService.h"

void UEVHttpService::SendGetRequest(const FString& Url, FEVHttpResponseDelegate Callback, float TimeoutSeconds)
{
    if (Url.IsEmpty())
    {
        Callback.ExecuteIfBound(false, 0, TEXT(""));
        return;
    }

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(Url);
    Request->SetVerb(TEXT("GET"));
    Request->SetTimeout(TimeoutSeconds);

    Request->OnProcessRequestComplete().BindUObject(this, &UEVHttpService::HandleResponse, Callback);

    Request->ProcessRequest();
}

void UEVHttpService::HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful,
                                    FEVHttpResponseDelegate Callback)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        Callback.ExecuteIfBound(false, 0, TEXT(""));
        return;
    }

    Callback.ExecuteIfBound(true, Response->GetResponseCode(), Response->GetContentAsString());
}