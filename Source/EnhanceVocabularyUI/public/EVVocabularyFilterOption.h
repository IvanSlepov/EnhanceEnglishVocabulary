#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVVocabularyFilterTypes.h"
#include "EVVocabularyFilterOption.generated.h"

class UButton;
class UTextBlock;
class UEVCheckBoxWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVocabularyFilterOptionSelectionChanged, EEVVocabularyFilterOption,
                                             Option, bool, bSelected);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVVocabularyFilterOption : public UUserWidget
{
    GENERATED_BODY()

public:
    void ConfigureOption(EEVVocabularyFilterOption InOption, bool bInSelected);
    void SetSelected(bool bInSelected, bool bBroadcastChange = false);
    bool IsSelected() const
    {
        return bSelected;
    }
    EEVVocabularyFilterOption GetOption() const
    {
        return Option;
    }

    UPROPERTY(BlueprintAssignable, Category = "Vocabulary Filters|Events")
    FOnVocabularyFilterOptionSelectionChanged OnSelectionChanged;

protected:
    virtual void NativeOnInitialized() override;

private:
    UFUNCTION()
    void HandleOptionButtonPressed();

    UFUNCTION()
    void HandleCheckStateChanged(bool bIsChecked);

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Option = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEVCheckBoxWidget> WBP_CheckBox_Option = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_OptionName = nullptr;

    UPROPERTY(Transient)
    EEVVocabularyFilterOption Option = EEVVocabularyFilterOption::None;

    UPROPERTY(Transient)
    bool bSelected = false;

    UPROPERTY(Transient)
    bool bApplyingState = false;
};
