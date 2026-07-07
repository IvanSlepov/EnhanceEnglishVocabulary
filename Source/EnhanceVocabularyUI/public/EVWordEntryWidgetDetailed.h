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
    class UButton* Button_EditWordEntry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_DeleteWordEntry;

    // Becomes Visible only when the Button_EditWord has been pressed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UButton* Button_SaveChanges;

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

    virtual void SetButtonsDisabled(bool bIsViewButtonDisabled, bool bIsEditButtonDisabled,
                                    bool bIsDeleteButtonDisabled, bool bIsSaveChangesButtonHidden) override;

    virtual void SetEditableFieldsReadOnly(bool bSetEnabled) override;

    /*Events*/

    // The following events are used to allow PC
    // to bind to this widget button events
    FSimpleMulticastDelegate OnViewRequested;
    FSimpleMulticastDelegate OnEditRequested;
    FOnWordEntryChangesSubmitted OnWordEntryChangesSubmitted;
    FSimpleMulticastDelegate OnDeleteRequested;

    virtual FSimpleMulticastDelegate& GetViewPressedDelegate() override
    {
        return OnViewRequested;
    }

    virtual FSimpleMulticastDelegate& GetEditPressedDelegate() override
    {
        return OnEditRequested;
    }

    virtual FOnWordEntryChangesSubmitted& GetSaveChangesSubmittedDelegate() override
    {
        return OnWordEntryChangesSubmitted;
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
    void HandleViewPressed();

    UFUNCTION()
    void HandleEditPressed();

    UFUNCTION()
    void HandleSaveChangesPressed();

    UFUNCTION()
    void HandleDeletePressed();
};
