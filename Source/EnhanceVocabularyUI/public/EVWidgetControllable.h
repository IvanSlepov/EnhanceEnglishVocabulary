#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVWidgetControllable.generated.h"

UINTERFACE(MinimalAPI)
class UEVWidgetControllable : public UInterface
{
    GENERATED_BODY()
};

class IEVWidgetControllable
{
    GENERATED_BODY()

public:
    virtual void SetControlsEnabled(bool bEnabled) = 0;
    virtual bool GetControlsEnabled() = 0;
};