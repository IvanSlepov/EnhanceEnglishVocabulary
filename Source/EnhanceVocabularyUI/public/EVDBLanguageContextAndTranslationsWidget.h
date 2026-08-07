#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "EVVocabularyLanguageTypes.h"
#include "EVDBLanguageContextAndTranslationsWidget.generated.h"

class UEVVocabularyTranslationsWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVocabularyLanguagePreferencesChanged,
                                            const FEVVocabularyLanguagePreferences&, Preferences);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVDBLanguageContextAndTranslationsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetInitialPreferences(const FEVVocabularyLanguagePreferences& InPreferences);
    const FEVVocabularyLanguagePreferences& GetWorkingPreferences() const
    {
        return WorkingPreferences;
    }

    UPROPERTY(BlueprintAssignable, Category = "Vocabulary Language|Events")
    FOnVocabularyLanguagePreferencesChanged OnPreferencesChanged;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

private:
    void PopulateDatabaseContextComboBox();
    void ApplyPreferencesToChildren();

    UFUNCTION()
    void HandleDatabaseContextSelected(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void HandleTranslationsApplied(const TArray<EEVVocabularyTranslationLanguage>& SelectedTranslations);

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UComboBoxString> ComboBoxString_DB_Language_Context = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEVVocabularyTranslationsWidget> WBP_VocabularyTranslationsWidget = nullptr;

    UPROPERTY(Transient)
    FEVVocabularyLanguagePreferences WorkingPreferences;

    UPROPERTY(Transient)
    bool bApplyingPreferences = false;
};
