// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "EVEntryItem.h"
#include "EVWordEntryDisplayWidgetProvider.h"
#include "Components/MultiLineEditableTextBox.h"
#include "EVWordEntryWidgetDetailed.generated.h"

/**
 *
 */
UCLASS()
class ENHANCEVOCABULARYUI_API UEVWordEntryWidgetDetailed : public UUserWidget, public IEVWordEntryDisplayWidgetProvider
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_ViewWord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_Word_Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_Word_Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_Definition_Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UMultiLineEditableTextBox* MultiLineEditableTextBox_Definition_Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_Usage_Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UMultiLineEditableTextBox* MultiLineEditableTextBox_Usage_Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_TranslationUA_Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UMultiLineEditableTextBox* MultiLineEditableTextBox_TranslationUA_Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_TranslationRU_Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UMultiLineEditableTextBox* MultiLineEditableTextBox_TranslationRU_Value;

    virtual void ShowWordEntry(const FVocabularyEntry& Entry) override;
    void SetEditableFieldsReadOnly(bool bSetEnabled);

    /*Events*/

    // The following events are used to allow PC
    // to bind to this widget button events
    FSimpleMulticastDelegate OnViewRequested;
    FSimpleMulticastDelegate OnEditRequested;
    FSimpleMulticastDelegate OnDeleteRequested;

    virtual FSimpleMulticastDelegate& GetViewPressedDelegate() override
    {
        return OnViewRequested;
    }
    virtual FSimpleMulticastDelegate& GetEditPressedDelegate() override
    {
        return OnEditRequested;
    }
    virtual FSimpleMulticastDelegate& GetDeletePressedDelegate() override
    {
        return OnDeleteRequested;
    }

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativePreConstruct() override;

private:
    UFUNCTION()
    void HandleViewClicked();

    UFUNCTION()
    void HandleEditClicked();

    UFUNCTION()
    void HandleDeleteClicked();
};
