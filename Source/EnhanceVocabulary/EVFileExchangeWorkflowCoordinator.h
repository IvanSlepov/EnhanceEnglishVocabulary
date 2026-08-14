#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVApplicationWorkflowSignals.h"
#include "EVFileExchangeTypes.h"
#include "EVFileExchangeWorkflowCoordinator.generated.h"

class UEVGameInstance;
class IEVVocabularyLibraryApplicationPort;
class IEVWidgetCommonEvents;

UCLASS()
class ENHANCEVOCABULARY_API UEVFileExchangeWorkflowCoordinator : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UEVGameInstance* InGameInstance, IEVVocabularyLibraryApplicationPort* InLibraryPort,
                    IEVWidgetCommonEvents* InLegacyEvents);
    void UpdatePresentationPorts(IEVVocabularyLibraryApplicationPort* InLibraryPort,
                                 IEVWidgetCommonEvents* InLegacyEvents);
    void RequestOperation(const FEVFileOperationInfo& Operation);
    void HandleOperationCompleted(const FEVRequestedActionInfo& Result);
    void HandleImportFilePickCompleted(const FEVFileExchangeResultInfo& Result);
    bool ResolveConfirmation(EEVConfirmationDialogType DialogType, bool bConfirmed);

    FOnEVWorkflowConfirmationRequested& OnConfirmationRequested()
    {
        return ConfirmationRequested;
    }
    FOnEVWorkflowLoadingChanged& OnLoadingChanged()
    {
        return LoadingChanged;
    }
    FOnEVWorkflowStatusRequested& OnStatusRequested()
    {
        return StatusRequested;
    }

private:
    UPROPERTY(Transient)
    TObjectPtr<UEVGameInstance> GameInstance;

    IEVVocabularyLibraryApplicationPort* LibraryPort = nullptr;
    IEVWidgetCommonEvents* LegacyEvents = nullptr;
    FEVFileOperationInfo PendingOperation;
    FOnEVWorkflowConfirmationRequested ConfirmationRequested;
    FOnEVWorkflowLoadingChanged LoadingChanged;
    FOnEVWorkflowStatusRequested StatusRequested;
};
