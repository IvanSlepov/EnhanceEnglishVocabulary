#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVApplicationRequestTypes.h"
#include "EVApplicationWorkflowSignals.h"
#include "EVVocabularyFilterTypes.h"
#include "EVVocabularyInteractionTypes.h"
#include "EVVocabularyTypes.h"
#include "EVVocabularyInteractionCoordinator.generated.h"

class UEVGameInstance;
class IEVEntryDetailsApplicationPort;
class IEVFeatureNavigationApplicationPort;
class IEVVocabularyFilterApplicationPort;
class IEVVocabularyLibraryApplicationPort;
class IEVWidgetCommonEvents;

UCLASS()
class ENHANCEVOCABULARY_API UEVVocabularyInteractionCoordinator : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UEVGameInstance* InGameInstance, IEVEntryDetailsApplicationPort* InEntryDetailsPort,
                    IEVVocabularyFilterApplicationPort* InFilterPort,
                    IEVFeatureNavigationApplicationPort* InNavigationPort,
                    IEVVocabularyLibraryApplicationPort* InLibraryPort, IEVWidgetCommonEvents* InLegacyEvents);
    void UpdatePresentationPorts(IEVEntryDetailsApplicationPort* InEntryDetailsPort,
                                 IEVVocabularyFilterApplicationPort* InFilterPort,
                                 IEVFeatureNavigationApplicationPort* InNavigationPort,
                                 IEVVocabularyLibraryApplicationPort* InLibraryPort,
                                 IEVWidgetCommonEvents* InLegacyEvents);

    void PresentEntryDetails(const FEVVocabularyRecord& Record);
    void PresentLegacyEntryDetails(const FEVWordEntryActionInfo& EntryAction);
    void RequestEntryDetailsClose();
    void RequestEntryDetailsSave(const FEVVocabularyRecord& Record);
    void RequestEntryDetailsDelete();
    void RequestVocabularyValueAction(const FEVVocabularyValueActionRequest& Request);
    void RequestVocabularyFilters();
    void ApplyVocabularyFilters(const FEVVocabularyQueryCriteria& Criteria);
    bool ResolveConfirmation(EEVConfirmationDialogType DialogType, bool bConfirmed);
    FString GetConfirmationSubject(EEVConfirmationDialogType DialogType) const;

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
    void ProcessConfirmedWordUpdate();
    void ProcessConfirmedWordDelete();
    void RequestRelationAction(const FEVVocabularyValueActionRequest& Request);
    void RequestTranslationAction(const FEVVocabularyValueActionRequest& Request);
    EEVVocabularyLanguageSupportState ResolveTranslationLanguageSupport(const FString& LanguageCode) const;
    bool DoesTranslationWordExistInTargetContext(const FEVVocabularyTranslation& Translation) const;
    void ApplyVocabularyChange(EEVVocabularyChangeType ChangeType, const FVocabularyEntry& Entry);

    UPROPERTY(Transient)
    TObjectPtr<UEVGameInstance> GameInstance;

    IEVEntryDetailsApplicationPort* EntryDetailsPort = nullptr;
    IEVVocabularyFilterApplicationPort* FilterPort = nullptr;
    IEVFeatureNavigationApplicationPort* NavigationPort = nullptr;
    IEVVocabularyLibraryApplicationPort* LibraryPort = nullptr;
    IEVWidgetCommonEvents* LegacyEvents = nullptr;

    FEVWordEntryActionInfo CurrentLegacyEntryAction;
    FEVVocabularyRecord CurrentRecord;
    FEVVocabularyRecord PendingRecord;
    FEVVocabularyValueActionRequest PendingValueAction;

    FOnEVWorkflowConfirmationRequested ConfirmationRequested;
    FOnEVWorkflowLoadingChanged LoadingChanged;
    FOnEVWorkflowStatusRequested StatusRequested;
};
