#pragma once

#include "EVConnectionTypesAndEnums.generated.h"

UENUM(BlueprintType)
enum class EEVConnectionState : uint8
{
    Offline,
    Connecting,
    Online
};