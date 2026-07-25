// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "EVPopUpSettingsTypes.h"
#include "EVPopUpSettingsWidget.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPopUpIntervalSelected, const FEVPopUpSettingsInfo&,
                                            EVPopUpSettingsInfo);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVPopUpSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UComboBoxString* ComboBoxString_PopUpIntervals;


    /*Events*/
    FOnPopUpIntervalSelected OnPopUpIntervalSelected;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:

    void PopulatePopUpIntervals();

    UFUNCTION()
    void HandleOnPopUpIntervalSelected(FString SelectedItem, ESelectInfo::Type SelectionType);
};
