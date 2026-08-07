#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVVocabularyLanguageTypes.h"
#include "EVVocabularyActiveTranslations.generated.h"

class UWrapBox;
class UEVVocabularyTranslationChip;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveTranslationRemoveRequested, EEVVocabularyTranslationLanguage,
                                            Language);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVVocabularyActiveTranslations : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetTranslations(const TArray<EEVVocabularyTranslationLanguage>& InTranslations);

    UPROPERTY(BlueprintAssignable, Category = "Vocabulary Translations|Events")
    FOnActiveTranslationRemoveRequested OnTranslationRemoveRequested;

private:
    void RebuildChips();

    UFUNCTION()
    void HandleChipRemoveRequested(EEVVocabularyTranslationLanguage Language);

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWrapBox> WrapBox_TranslationOptions = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vocabulary Translations|Widget Classes",
              meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UEVVocabularyTranslationChip> TranslationChipWidgetClass;

    UPROPERTY(Transient)
    TArray<EEVVocabularyTranslationLanguage> Translations;
};
