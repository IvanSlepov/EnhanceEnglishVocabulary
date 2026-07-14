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
#include "EVAppSettingsWidget.h"
#include "EVImportExportDBWidget.h"
#include "EVErrorProvider.h"
#include "EVErrorTypes.h"
#include "EVRequestedActionTypes.h"
#include "EVConnectionTypesAndEnums.h"
#include "EVWidgetCommonEvents.h"
#include "EVWordEntryActionTypes.h"
#include "EVFileExchangeTypes.h"
#include "EVRootWidget.generated.h"

/**
 *
 */

UCLASS()
class ENHANCEVOCABULARYUI_API UEVRootWidget : public UUserWidget, public IEVErrorProvider, public IEVWidgetCommonEvents
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

    /*The EV app WBPs added to the WidgetSwitcher_Main*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget), Category = "Online Dependant")
    TObjectPtr<UEVAddWordWidget> AddWord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UEVNoMenuWidget> NoMenu;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UEVMainMenuWidget> MainMenu;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UEVReviewWordsWidget> ReviewWords;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UEVAppSettingsWidget> Settings_SelectWebProviders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UEVImportExportDBWidget> ImportExportDB;

    class UEVGameInstance* EVGameInstance;

    /*Events*/

    // Interface derrived event declaration
    virtual FOnEVError& GetOnErrorEvent() override
    {
        return OnRootWidgetError;
    }

    virtual FOnLoadingDataTriggerred* GetLoadingSpinnerEvent() override
    {
        return &OnLoadingDataTriggerred;
    }

    virtual FOnActionRequested* GetRequestedActionInfo() override
    {
        return &OnActionRequested;
    }

    virtual FOnWordEntryWidgetControlsActivated* GetCurrentWordEntryWidgetActionInfo() override
    {
        return &OnWordEntryWidgetControlsActivated;
    }

    virtual FOnImportExportDownloadDBOperationIssued* GetIssuedFileOperationInfo() override
    {
        return &OnImportExportDownloadDBOperationIssued;
    }

    virtual void HandleWordEntryChanged(const FEVWordEntryActionInfo& WordEntryActionInfo) override;

    virtual void HandleReviewWordsRefresh() override;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    void SetupConnectionErrorInfo(FEVErrorInfo& ConnectionErrorInfo);
    bool HandleWidgetControlsState(IEVWidgetControllable* Widget, bool bIsConnectionStatusOnline);
    void HandleOnlineDependantWidgetsActivation(UUserWidget* Widget, bool bIsConnectionStatusOnline);

    UFUNCTION()
    void ButtonMenuPressed();

    UFUNCTION()
    void HandleMenuButtonsPressed(bool bIsAddWordActivated, bool bIsReviewWordsActivated,
                                  bool bIsPopupSettingsActivated, bool bIsImportExportActivated,
                                  bool bIsAppSettingsActivated);

    bool bIsAddWordActivated_internal;
    bool bIsReviewWordsActivated_internal;
    bool bIsPopupSettingsActivated_internal;
    bool bIsImportExportActivated_internal;
    bool bIsAppSettingsActivated_internal;

    UFUNCTION()
    void HandleQuitButtonPressed();

    UFUNCTION()
    void HandleOnAnyWidgetErrorDetected(const FEVErrorInfo& WidgetErrorInfo);

    // We use this handler to invoke the ErrorWidget
    // if any connection issues are detected
    // Interface derrived event
    UPROPERTY(BlueprintAssignable)
    FOnEVError OnRootWidgetError;

    UPROPERTY(BlueprintAssignable)
    FOnLoadingDataTriggerred OnLoadingDataTriggerred;

    UPROPERTY(BlueprintAssignable)
    FOnActionRequested OnActionRequested;

    UPROPERTY(BlueprintAssignable)
    FOnWordEntryWidgetControlsActivated OnWordEntryWidgetControlsActivated;

    UPROPERTY(BlueprintAssignable)
    FOnImportExportDownloadDBOperationIssued OnImportExportDownloadDBOperationIssued;

    UFUNCTION()
    void HandleOnConnectionErrorDetected();

    // Use this to handle an event fired back from any appropriate widget
    // signaling that the controls have been disabled and further generate the appropriate error widget
    UFUNCTION()
    void HandleOnAnyWidgetControlsDisabled(bool bAreControlsEnabled, const FString& WidgetName);

    UFUNCTION()
    void HandleOnConnectionStateChanged(EEVConnectionState State);

    UFUNCTION()
    void HandleLoadingSpinner(bool bRenderLoadingSpinner);

    UFUNCTION()
    void HandleOnActionRequested(const FEVRequestedActionInfo& RequestedActionInfo);

    UFUNCTION()
    void HandleOnWordEntryWidgetControlsActivated(const FEVWordEntryActionInfo& WordEntryActionInfo);

    UFUNCTION()
    void HandleOnImportExportDownloadDBOperationIssued(const FEVFileOperationInfo& FileOperationInfoFromSelectorWidget);

    bool bIsAnyMenuActivated;
    int32 MenuSwitcherCount;
    bool bIsAppOnline = false;

    FEVErrorInfo EVConnectionErrorInfo;

    EEVConnectionState EVConnectionState;

    // Connection state color
    static const FName SphereColorParam;
    static const FName OpacityParam;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> ConnectionMID = nullptr;

    void HandleConnectionImageColor(TObjectPtr<UMaterialInstanceDynamic> MaterialInstanceDynamic,
                                    EEVConnectionState ConnectionState);
};
