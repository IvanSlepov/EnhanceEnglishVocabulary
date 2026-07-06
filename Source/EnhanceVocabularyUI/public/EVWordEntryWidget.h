// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/SlateWrapperTypes.h"
#include "EVWordEntryDisplayWidgetProvider.h"
#include "EVEntryItem.h"
#include "EVWordEntryWidget.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnWordEntryButtonPressed, UEVWordEntryWidget*, WordEntryWidget, bool,
                                              bIsViewButtonPressed, bool, bIsEditButtonPressed, bool,
                                              bIsDeleteButtonPressed, bool, bIsOkButtonPressed);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVWordEntryWidget : public UUserWidget,
                                                   public IUserObjectListEntry,
                                                   public IEVWordEntryDisplayWidgetProvider
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class USizeBox* SizeBox_WordEntry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_ViewWord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UHorizontalBox* HorizontalBox_Definition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UHorizontalBox* HorizontalBox_Usage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UHorizontalBox* HorizontalBox_TranslationUA;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UHorizontalBox* HorizontalBox_TranslationRU;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_Word_Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_Word_Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_Definition_Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_Definition_Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_Usage_Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_Usage_Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_TranslationUA_Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_TranslationUA_Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_TranslationRU_Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_TranslationRU_Value;

    void ExpandVisual();
    void CollapseVisual();

    // Used to be called or bound from the PlayerController
    virtual void ShowWordEntry(const FVocabularyEntry& Entry) override;
    virtual void SetHorizontalBoxSizeFill() override;
    virtual void ViewButtonPressedFromPlayerController();

    UPROPERTY(BlueprintAssignable)
    FOnWordEntryButtonPressed OnWordEntryButtonPressed;

    FVocabularyEntry GetCurrentWidgetVocabularyEnryItemInfo();

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

    // Triggerred by the ListItemObject, automatically
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

private:
    bool bAreTextFieldsCreated;

    FVector2D SizeBoxInitialSize;

    UFUNCTION()
    void HandleOnWordEntry_ViewButtonPressed();

    UFUNCTION()
    void HandleOnWordEntry_EditButtonPressed();

    UFUNCTION()
    void HandleOnWordEntry_DeleteButtonPressed();

    UFUNCTION()
    void HandleOnWordEntry_OkButtonPressed();

    FVocabularyEntry CurrentWidgetEnryItemToDisplayInPCGeneratedWordEntry;

    bool bAreHorizontalBoxesInitialized;
    bool bCalledFromPlayerController = false;

    void SetBoxFill(UHorizontalBox* Box, float FillValue = 1.0f);
};
