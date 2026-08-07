#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVVocabularyLanguageTypes.h"
#include "EVVocabularyTranslationOption.generated.h"

class UButton;
class UTextBlock;
class UEVCheckBoxWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVocabularyTranslationOptionSelectionChanged,
                                             EEVVocabularyTranslationLanguage, Language, bool, bSelected);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVVocabularyTranslationOption : public UUserWidget
{
    GENERATED_BODY()

public:
    void ConfigureOption(EEVVocabularyTranslationLanguage InLanguage, bool bInSelected);
    void SetSelected(bool bInSelected, bool bBroadcastChange = false);

    EEVVocabularyTranslationLanguage GetLanguage() const
    {
        return Language;
    }
    bool IsSelected() const
    {
        return bSelected;
    }

    UPROPERTY(BlueprintAssignable, Category = "Vocabulary Translations|Events")
    FOnVocabularyTranslationOptionSelectionChanged OnSelectionChanged;

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
    EEVVocabularyTranslationLanguage Language = EEVVocabularyTranslationLanguage::None;

    UPROPERTY(Transient)
    bool bSelected = false;

    UPROPERTY(Transient)
    bool bApplyingState = false;
};
