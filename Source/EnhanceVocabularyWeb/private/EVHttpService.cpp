// Fill out your copyright notice in the Description page of Project Settings.

#include "EVHttpService.h"

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
constexpr TCHAR FreeDictionaryApiBaseUrl[] = TEXT("https://api.dictionaryapi.dev/api/v2/entries/en/");
}

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

    // Uncomment only when the complete Free Dictionary API response is needed.
    DebugDictionaryApiResponse(Request, Response);

    Callback.ExecuteIfBound(true, Response->GetResponseCode(), Response->GetContentAsString());
}

void UEVHttpService::DebugDictionaryApiResponse(const FHttpRequestPtr& Request, const FHttpResponsePtr& Response) const
{
    if (!Request.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("DebugDictionaryApiResponse: Request is invalid."));

        return;
    }

    if (!Response.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("DebugDictionaryApiResponse: Response is invalid."));

        return;
    }

    const FString RequestUrl = Request->GetURL();

    // This HTTP service is shared by multiple providers.
    // Ignore every response except the Free Dictionary API response.
    if (!RequestUrl.StartsWith(FreeDictionaryApiBaseUrl, ESearchCase::IgnoreCase))
    {
        return;
    }

    const int32 ResponseCode = Response->GetResponseCode();

    const FString ResponseContent = Response->GetContentAsString();

    UE_LOG(LogTemp, Display,
           TEXT("Free Dictionary API debug response:\n"
                "URL: %s\n"
                "Response code: %d\n"
                "Content type: %s\n"
                "Content length: %d"),
           *RequestUrl, ResponseCode, *Response->GetContentType(), ResponseContent.Len());

    UE_LOG(LogTemp, Warning, TEXT("Free Dictionary API response body:\n%s"), *ResponseContent);

    const FString DebugDirectory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Debug"));

    const bool bDirectoryExistsOrWasCreated = IFileManager::Get().MakeDirectory(*DebugDirectory, true);

    if (!bDirectoryExistsOrWasCreated)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create debug directory: %s"), *DebugDirectory);

        return;
    }

    const FString DebugFilePath = FPaths::Combine(DebugDirectory, TEXT("FreeDictionaryApiResponse.json"));

    const bool bResponseSaved = FFileHelper::SaveStringToFile(ResponseContent, *DebugFilePath,
                                                              FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

    if (bResponseSaved)
    {
        UE_LOG(LogTemp, Display, TEXT("Complete Free Dictionary API response saved to: %s"), *DebugFilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save Free Dictionary API response to: %s"), *DebugFilePath);
    }
}