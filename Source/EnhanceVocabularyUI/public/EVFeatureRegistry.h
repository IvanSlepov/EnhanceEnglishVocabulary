#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVFeatureRegistry.generated.h"

class UUserWidget;
class UWidgetSwitcher;

UCLASS()
class ENHANCEVOCABULARYUI_API UEVFeatureRegistry : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UWidgetSwitcher* InSwitcher);
    bool RegisterFeature(FName FeatureId, UUserWidget* FeatureWidget);
    bool IsFeatureAvailable(FName FeatureId) const;
    bool ActivateFeature(FName FeatureId, bool bRememberCurrent = true);
    bool ReturnToPreviousFeature();
    FName GetActiveFeatureId() const
    {
        return ActiveFeatureId;
    }

private:
    TWeakObjectPtr<UWidgetSwitcher> Switcher;
    TMap<FName, TWeakObjectPtr<UUserWidget>> Features;
    FName ActiveFeatureId = NAME_None;
    FName PreviousFeatureId = NAME_None;
};
