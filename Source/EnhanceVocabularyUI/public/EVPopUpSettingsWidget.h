// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "EVPopUpSettingsTypes.h"
#include "EVFeatureRoles.h"
#include "EVPopUpSettingsWidget.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotificationSettingsChanged, const FEVPopUpSettingsInfo&,
                                            EVPopUpSettingsInfo);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVPopUpSettingsWidget : public UUserWidget, public IEVNotificationSettingsFeatureRole
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UComboBoxString* ComboBoxString_NotificationIntervals;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UComboBoxString* ComboBoxString_NotificationModes;

    void SetSelectedSettings(const FEVPopUpSettingsInfo& Settings);

    /*Events*/
    FOnNotificationSettingsChanged OnNotificationSettingsChanged;

    virtual FOnEVFeatureNotificationSettingsChanged& GetNotificationSettingsChangedEvent() override
    {
        return OnFeatureNotificationSettingsChanged;
    }

    virtual void ApplyNotificationSettings(const FEVPopUpSettingsInfo& Settings) override
    {
        SetSelectedSettings(Settings);
    }

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    FOnEVFeatureNotificationSettingsChanged OnFeatureNotificationSettingsChanged;

    bool bApplyingSettingsFromController = false;

    void PopulateNotificationIntervals();
    void PopulateNotificationModes();

    void BroadcastCurrentSettings();

    FEVPopUpSettingsInfo EVNotificationSettingsInfo;

    UFUNCTION()
    void HandleOnNotificationsIntervalSelected(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void HandleOnNotificationsModeSelected(FString SelectedItem, ESelectInfo::Type SelectionType);
};
