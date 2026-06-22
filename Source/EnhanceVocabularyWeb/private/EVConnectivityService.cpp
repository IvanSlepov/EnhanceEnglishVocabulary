// Fill out your copyright notice in the Description page of Project Settings.

#include "EVConnectivityService.h"

bool UEVConnectivityService::Initialize()
{
    ConnectionState = EEVConnectionState::Offline;
    return true;
}

void UEVConnectivityService::Shutdown()
{
    ConnectionState = EEVConnectionState::Offline;
}

void UEVConnectivityService::RefreshConnection()
{
    ConnectionState = EEVConnectionState::Connecting;

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(TEXT("https://clients3.google.com/generate_204"));
    Request->SetVerb(TEXT("GET"));
    Request->SetTimeout(10.0f);

    Request->OnProcessRequestComplete().BindUObject(this, &UEVConnectivityService::HandleConnectionResponse);

    Request->ProcessRequest();
}

EEVConnectionState UEVConnectivityService::GetConnectionState() const
{
    return ConnectionState;
}

void UEVConnectivityService::StartConnectionPolling(UWorld* World, float IntervalSeconds)
{
    // ConnectionState polling
    if (World)
    {
        World->GetTimerManager().SetTimer(RefreshConnectionStatusHandle, this,
                                          &UEVConnectivityService::RefreshConnection, IntervalSeconds, true);
    }
}

void UEVConnectivityService::StopConnectionPolling(UWorld* World)
{
    if (World)
    {
        World->GetTimerManager().ClearTimer(RefreshConnectionStatusHandle);
    }
}

void UEVConnectivityService::HandleConnectionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response,
                                                      bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        const int32 Code = Response->GetResponseCode();

        if (Code == 204 || (Code >= 200 && Code < 300))
        {
            ConnectionState = EEVConnectionState::Online;
            UE_LOG(LogTemp, Warning, TEXT("Connection state: Online"));
        }
        else
        {
            ConnectionState = EEVConnectionState::Offline;
            UE_LOG(LogTemp, Warning, TEXT("Connection state: Offline"));
        }

        OnConnectionStateChanged.Broadcast(ConnectionState);
        return;
    }

    ConnectionState = EEVConnectionState::Offline;
    UE_LOG(LogTemp, Warning, TEXT("Connection state: Offline"));
    OnConnectionStateChanged.Broadcast(ConnectionState);
}
