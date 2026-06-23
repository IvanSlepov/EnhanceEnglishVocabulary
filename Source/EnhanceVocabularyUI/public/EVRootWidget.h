// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Widgets/SWidget.h"
#include "Components/PanelWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/WidgetSwitcherSlot.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EVAddWordWidget.h"
#include "EVMainMenuWidget.h"
#include "EVNoMenuWidget.h"
#include "EVReviewWordsWidget.h"
#include "EVErrorProvider.h"
#include "EVConnectionTypesAndEnums.h"
#include "EVRootWidget.generated.h"

/**
 *
 */

UCLASS()
class ENHANCEVOCABULARYUI_API UEVRootWidget : public UUserWidget, public IEVErrorProvider
{
    GENERATED_BODY()

public:
    // Native UE5.7 widgets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_Menu;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UImage> Image_ConnectionState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UWidgetSwitcher> WidgetSwitcher_Main;

    // The EV app WBPs added to the WidgetSwitcher_Main
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UEVAddWordWidget> AddWord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UEVNoMenuWidget> NoMenu;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UEVMainMenuWidget> MainMenu;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UEVReviewWordsWidget> ReviewWords;

    class UEVGameInstance* EVGameInstance;

    /*Events*/

    // Interface derrived even declaration
    virtual FOnEVError& GetOnErrorEvent() override
    {
        return OnRootWidgetError;
    }

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    UFUNCTION()
    void ButtonMenuPressed();

    UFUNCTION()
    void HandleMenuButtonsPressed(bool bIsAddWordActivated, bool bIsReviewWordsActivated,
                                  bool bIsPopupSettingsActivated, bool bIsImportExportActivated);

    bool bIsAddWordActivated_internal;
    bool bIsReviewWordsActivated_internal;
    bool bIsPopupSettingsActivated_internal;
    bool bIsImportExportActivated_internal;

    UFUNCTION()
    void HandleQuitButtonPressed();

    UFUNCTION()
    void HandleOnAnyWidgedErrorDetected(const FEVErrorInfo& WidgetErrorInfo);

    UFUNCTION()
    void HandleOnConnectionStateChanged(EEVConnectionState State);

    bool bIsAnyMenuActivated;
    int32 MenuSwitcherCount;
    bool bIsAppOnline = false;

    // Interface derrived event
    UPROPERTY(BlueprintAssignable)
    FOnEVError OnRootWidgetError;

    EEVConnectionState EVConnectionState;

    // Connection state color
    static const FName SphereColorParam;
    static const FName OpacityParam;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> ConnectionMID = nullptr;

    void HandleConnectionImageColor(TObjectPtr<UMaterialInstanceDynamic> MaterialInstanceDynamic,
                                    EEVConnectionState ConnectionState);
};
