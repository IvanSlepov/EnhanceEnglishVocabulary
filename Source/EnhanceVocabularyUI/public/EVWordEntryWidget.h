// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "EVEntryItem.h"
#include "EVWordEntryWidget.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWordEntryViewButtonPressed, UEVWordEntryWidget*, WordEntryWidget);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVWordEntryWidget : public UUserWidget, public IUserObjectListEntry
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
    class UTextBlock* TextBlock_Transcription_Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TextBlock_Transcription_Value;

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

    UPROPERTY(BlueprintAssignable)
    FOnWordEntryViewButtonPressed OnWordEntryViewButtonPressed;

    FVocabularyEntry GetCurrentWidgetVocabularyEnryItemInfo();

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

    // Triggerred by the ListItemObject, automatically
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

    FVocabularyEntry CurrentWidgetEnryItemToDisplayInPCGeneratedWordEntry;

private:
    bool bAreTextFieldsCreated;

    UFUNCTION()
    void HandleOnWordEntry_ViewButtonPressed();
};
