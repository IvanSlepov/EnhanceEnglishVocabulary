#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVVocabularyLanguageTypes.h"
#include "EVVocabularyTranslationsWidget.generated.h"

class UBorder;
class UButton;
class UTextBlock;
class UVerticalBox;
class UEVVocabularyTranslationOption;
class UEVVocabularyActiveTranslations;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVocabularyTranslationsApplied,
                                            const TArray<EEVVocabularyTranslationLanguage>&, SelectedTranslations);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVVocabularyTranslationsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetInitialState(EEVVocabularyDBContext InContext,
                         const TArray<EEVVocabularyTranslationLanguage>& InSelectedTranslations);

    void SetDatabaseContext(EEVVocabularyDBContext InContext);

    const TArray<EEVVocabularyTranslationLanguage>& GetWorkingTranslations() const
    {
        return WorkingTranslations;
    }

    UPROPERTY(BlueprintAssignable, Category = "Vocabulary Translations|Events")
    FOnVocabularyTranslationsApplied OnTranslationsApplied;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

private:
    void NormalizeWorkingTranslations();
    void PopulateTranslationOptions();
    void RebuildActiveTranslations();
    void SynchronizeOptionWidgets();
    void UpdateEmptyState();

    UFUNCTION()
    void HandleClosePressed();

    UFUNCTION()
    void HandleTranslationSelectorPressed();

    UFUNCTION()
    void HandleApplyChangesPressed();

    UFUNCTION()
    void HandleTranslationOptionSelectionChanged(EEVVocabularyTranslationLanguage Language, bool bSelected);

    UFUNCTION()
    void HandleActiveTranslationRemoveRequested(EEVVocabularyTranslationLanguage Language);

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Close = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_TranslationSelector = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> Border_TranslationsPanel = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> VerticalBox_Translations = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_NoActiveTranslations = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> VerticalBox_ActiveTranslations = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_ApplyChanges = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vocabulary Translations|Widget Classes",
              meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UEVVocabularyTranslationOption> TranslationOptionWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vocabulary Translations|Widget Classes",
              meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UEVVocabularyActiveTranslations> ActiveTranslationsWidgetClass;

    UPROPERTY(Transient)
    EEVVocabularyDBContext DatabaseContext = EEVVocabularyDBContext::EnglishUSA;

    UPROPERTY(Transient)
    TArray<EEVVocabularyTranslationLanguage> CommittedTranslations;

    UPROPERTY(Transient)
    TArray<EEVVocabularyTranslationLanguage> WorkingTranslations;

    UPROPERTY(Transient)
    bool bTranslationsExpanded = false;

    TMap<EEVVocabularyTranslationLanguage, UEVVocabularyTranslationOption*> TranslationOptionWidgets;
};
