// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Widgets/SWidget.h"
#include "Components/PanelWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/WidgetSwitcherSlot.h"
#include "EVAddWordWidget.h"
#include "EVMainMenuWidget.h"
#include "EVNoMenuWidget.h"
#include "EVReviewWordsWidget.h"
#include "EVRootWidget.generated.h"

/**
 *
 */
UCLASS()
class ENHANCEVOCABULARYUI_API UEVRootWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Native UE5.7 widgets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_Menu;

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

    bool bIsAnyMenuActivated;
    int32 MenuSwitcherCount;
};
