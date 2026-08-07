#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVVocabularyLanguageTypes.h"
#include "EVVocabularyTranslationChip.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UEVVocabularyLanguageUiData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVocabularyTranslationChipRemoveRequested,
                                            EEVVocabularyTranslationLanguage, Language);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVVocabularyTranslationChip : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetLanguage(EEVVocabularyTranslationLanguage InLanguage);
    EEVVocabularyTranslationLanguage GetLanguage() const
    {
        return Language;
    }

    UPROPERTY(BlueprintAssignable, Category = "Vocabulary Translations|Events")
    FOnVocabularyTranslationChipRemoveRequested OnRemoveRequested;

protected:
    virtual void NativeOnInitialized() override;

private:
    void ApplyLanguageVisuals();

    UFUNCTION()
    void HandleRemovePressed();

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> Image_LanguageCountryFlag = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_Value = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Remove = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vocabulary Translations|Presentation",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UEVVocabularyLanguageUiData> LanguageUiData = nullptr;

    UPROPERTY(Transient)
    EEVVocabularyTranslationLanguage Language = EEVVocabularyTranslationLanguage::None;
};
