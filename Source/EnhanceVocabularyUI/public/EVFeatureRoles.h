#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVApplicationFeatureTypes.h"
#include "EVApplicationRequestTypes.h"
#include "EVApplicationLifecyclePort.h"
#include "EVErrorProvider.h"
#include "EVFileExchangeTypes.h"
#include "EVPopUpSettingsTypes.h"
#include "EVRequestedActionTypes.h"
#include "EVVocabularyFilterTypes.h"
#include "EVVocabularyInteractionTypes.h"
#include "EVVocabularyLanguageTypes.h"
#include "EVVocabularyTypes.h"
#include "EVWebProviderTypes.h"
#include "EVWidgetCommonEvents.h"
#include "EVFeatureRoles.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVFeatureNavigationRequested, FName);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEVWebProviderSelectionChanged, EEVWebProvider, EEVWebProvider);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVFeatureEntryDetailsRequested, const FEVVocabularyRecord&);
DECLARE_MULTICAST_DELEGATE(FOnEVFeatureFiltersRequested);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVFeatureVocabularyValueActionRequested, const FEVVocabularyValueActionRequest&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVFeatureFileOperationRequested, const FEVFileOperationInfo&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEVFeatureNotificationSettingsChanged, const FEVPopUpSettingsInfo&);

UINTERFACE(MinimalAPI)
class UEVFeatureFeedbackSource : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYUI_API IEVFeatureFeedbackSource
{
    GENERATED_BODY()

public:
    virtual FOnEVError* GetFeatureErrorEvent()
    {
        return nullptr;
    }
    virtual FOnWidgetInteractionDisabled* GetFeatureInteractionDisabledEvent()
    {
        return nullptr;
    }
    virtual FOnLoadingDataTriggerred* GetFeatureLoadingStateEvent()
    {
        return nullptr;
    }
    virtual FOnActionRequested* GetFeatureStatusEvent()
    {
        return nullptr;
    }
    virtual void ApplyFeatureErrorResolution(const FEVErrorInfo&) {}
};

UINTERFACE(MinimalAPI)
class UEVFeatureLifecycleRole : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYUI_API IEVFeatureLifecycleRole
{
    GENERATED_BODY()

public:
    virtual void ApplyFeatureActivationState(bool bIsActive) = 0;
};

UINTERFACE(MinimalAPI)
class UEVMainMenuFeatureRole : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYUI_API IEVMainMenuFeatureRole
{
    GENERATED_BODY()

public:
    virtual FOnEVFeatureNavigationRequested& GetFeatureNavigationRequestedEvent() = 0;
    virtual FOnEVApplicationExitRequested& GetApplicationExitRequestedEvent() = 0;
    virtual void SetFeatureAvailable(FName FeatureId, bool bAvailable) = 0;
};

UINTERFACE(MinimalAPI)
class UEVWebProviderSelectionSource : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYUI_API IEVWebProviderSelectionSource
{
    GENERATED_BODY()

public:
    virtual FOnEVWebProviderSelectionChanged& GetWebProviderSelectionChangedEvent() = 0;
};

UINTERFACE(MinimalAPI)
class UEVWebProviderSelectionConsumer : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYUI_API IEVWebProviderSelectionConsumer
{
    GENERATED_BODY()

public:
    virtual void ApplyWebProviderSelection(EEVWebProvider DefinitionProvider, EEVWebProvider TranslationProvider) = 0;
};

UINTERFACE(MinimalAPI)
class UEVVocabularyPreferencesFeatureRole : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYUI_API IEVVocabularyPreferencesFeatureRole
{
    GENERATED_BODY()

public:
    virtual void ApplyVocabularyPreferences(const FEVVocabularyLanguagePreferences& Preferences) = 0;
};

UINTERFACE(MinimalAPI)
class UEVWordContextFeatureRole : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYUI_API IEVWordContextFeatureRole
{
    GENERATED_BODY()

public:
    virtual void PresentWordContext(const FString& Word) = 0;
};

UINTERFACE(MinimalAPI)
class UEVReviewFeatureRole : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYUI_API IEVReviewFeatureRole
{
    GENERATED_BODY()

public:
    virtual FOnEVFeatureEntryDetailsRequested& GetEntryDetailsRequestedEvent() = 0;
    virtual FOnEVFeatureFiltersRequested& GetFiltersRequestedEvent() = 0;
    virtual FOnEVFeatureVocabularyValueActionRequested& GetVocabularyValueActionRequestedEvent() = 0;
    virtual void ApplyQueryCriteria(const FEVVocabularyQueryCriteria& Criteria) = 0;
    virtual void ApplyVocabularyChange(const FEVVocabularyChangeInfo& ChangeInfo) = 0;
    virtual void RefreshFeature() = 0;
};

UINTERFACE(MinimalAPI)
class UEVFileExchangeFeatureRole : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYUI_API IEVFileExchangeFeatureRole
{
    GENERATED_BODY()

public:
    virtual FOnEVFeatureFileOperationRequested& GetFileOperationRequestedEvent() = 0;
};

UINTERFACE(MinimalAPI)
class UEVNotificationSettingsFeatureRole : public UInterface
{
    GENERATED_BODY()
};

class ENHANCEVOCABULARYUI_API IEVNotificationSettingsFeatureRole
{
    GENERATED_BODY()

public:
    virtual FOnEVFeatureNotificationSettingsChanged& GetNotificationSettingsChangedEvent() = 0;
    virtual void ApplyNotificationSettings(const FEVPopUpSettingsInfo& Settings) = 0;
};
