// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVConnectionTypesAndEnums.h"
#include "Http.h"
#include "Engine/TimerHandle.h"

#include "EVConnectivityService.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEVConnectivityStateChanged, EEVConnectionState, NewState);

UCLASS()
class ENHANCEVOCABULARYWEB_API UEVConnectivityService : public UObject
{
    GENERATED_BODY()

public:
    bool Initialize();

    void Shutdown();

    UFUNCTION(BlueprintCallable, Category = "Connectivity")
    void RefreshConnection();

    EEVConnectionState GetConnectionState() const;

    void StartConnectionPolling(UWorld* World, float IntervalSeconds = 10.0f);
    void StopConnectionPolling(UWorld* World);

    UPROPERTY(BlueprintAssignable)
    FEVConnectivityStateChanged OnConnectionStateChanged;

private:
    EEVConnectionState ConnectionState = EEVConnectionState::Offline;

    void HandleConnectionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    FTimerHandle RefreshConnectionStatusHandle;
};
