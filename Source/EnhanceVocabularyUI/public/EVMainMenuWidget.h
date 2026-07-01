// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "EVMainMenuWidget.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnMenuButtonsPressed, bool, IsAddWordActivated, bool,
                                              IsReviewWordsActivated, bool, IsPopupSettingsActivated, bool,
                                              IsImportExportActivated, bool, IsAppSettingsActivated);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuitButtonPressed);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Each button represents either a separate EV app's UI Widget
    // or the direct functionality i.e., QuitButton
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_AddWord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_ReviewWords;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_PopupSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_ImportExport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_AppSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_Quit;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Main Menu Events")
    FOnMenuButtonsPressed OnMenuButtonsPressed;

    UPROPERTY(BlueprintAssignable, Category = "Main Menu Events")
    FOnQuitButtonPressed OnQuitButtonPressed;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    bool bIsAddWordActivated;
    bool bIsReviewWordsActivated;
    bool bIsPopupSettingsActivated;
    bool bIsImportExportActivated;
    bool bIsAppSettingsActivated;

    UFUNCTION()
    void HandleAddWordButtonPressed();

    UFUNCTION()
    void HandleReviewWordsButtonPressed();

    UFUNCTION()
    void HandlePopupSettingsButtonPressed();

    UFUNCTION()
    void HandleImportExportButtonPressed();

    UFUNCTION()
    void HandleAppSettingsButtonPressed();

    UFUNCTION()
    void HandleQuitButtonPressed();
};
