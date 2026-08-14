#include "EVFeatureRegistry.h"

#include "Blueprint/UserWidget.h"
#include "Components/WidgetSwitcher.h"
#include "EVFeatureRoles.h"

void UEVFeatureRegistry::Initialize(UWidgetSwitcher* InSwitcher)
{
    Switcher = InSwitcher;
    Features.Reset();
    ActiveFeatureId = NAME_None;
    PreviousFeatureId = NAME_None;
}

bool UEVFeatureRegistry::RegisterFeature(const FName FeatureId, UUserWidget* FeatureWidget)
{
    if (FeatureId.IsNone() || !FeatureWidget)
    {
        return false;
    }

    Features.Add(FeatureId, FeatureWidget);
    return true;
}

bool UEVFeatureRegistry::IsFeatureAvailable(const FName FeatureId) const
{
    const TWeakObjectPtr<UUserWidget>* Feature = Features.Find(FeatureId);
    return Feature && Feature->IsValid();
}

bool UEVFeatureRegistry::ActivateFeature(const FName FeatureId, const bool bRememberCurrent)
{
    if (!Switcher.IsValid())
    {
        return false;
    }

    const TWeakObjectPtr<UUserWidget>* Feature = Features.Find(FeatureId);
    if (!Feature || !Feature->IsValid())
    {
        return false;
    }

    const FName PreviouslyActiveFeatureId = ActiveFeatureId;
    if (bRememberCurrent && !PreviouslyActiveFeatureId.IsNone() && PreviouslyActiveFeatureId != FeatureId)
    {
        PreviousFeatureId = PreviouslyActiveFeatureId;
    }

    if (PreviouslyActiveFeatureId != FeatureId)
    {
        if (const TWeakObjectPtr<UUserWidget>* PreviouslyActiveFeature = Features.Find(PreviouslyActiveFeatureId))
        {
            if (IEVFeatureLifecycleRole* LifecycleRole = Cast<IEVFeatureLifecycleRole>(PreviouslyActiveFeature->Get()))
            {
                LifecycleRole->ApplyFeatureActivationState(false);
            }
        }
    }

    Switcher->SetActiveWidget(Feature->Get());
    ActiveFeatureId = FeatureId;

    if (PreviouslyActiveFeatureId != FeatureId)
    {
        if (IEVFeatureLifecycleRole* LifecycleRole = Cast<IEVFeatureLifecycleRole>(Feature->Get()))
        {
            LifecycleRole->ApplyFeatureActivationState(true);
        }
    }

    return true;
}

bool UEVFeatureRegistry::ReturnToPreviousFeature()
{
    if (PreviousFeatureId.IsNone())
    {
        return false;
    }

    const FName Destination = PreviousFeatureId;
    PreviousFeatureId = NAME_None;
    return ActivateFeature(Destination, false);
}
