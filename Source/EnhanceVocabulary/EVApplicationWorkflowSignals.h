#pragma once

#include "CoreMinimal.h"
#include "EVConfirmationDialogActionTypes.h"
#include "EVRequestedActionTypes.h"
#include "EVWordEntryActionTypes.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEVWorkflowConfirmationRequested, EEVConfirmationDialogType,
                                     EEVWordEntryActionType);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVWorkflowLoadingChanged, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVWorkflowStatusRequested, const FEVRequestedActionInfo&);
