#include "EVDBLanguageContextAndTranslationsWidget.h"

#include "Components/ComboBoxString.h"
#include "EVVocabularyTranslationsWidget.h"

void UEVDBLanguageContextAndTranslationsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (ComboBoxString_DB_Language_Context)
    {
        ComboBoxString_DB_Language_Context->OnSelectionChanged.AddUniqueDynamic(
            this, &ThisClass::HandleDatabaseContextSelected);
    }

    if (WBP_VocabularyTranslationsWidget)
    {
        WBP_VocabularyTranslationsWidget->OnTranslationsApplied.AddUniqueDynamic(this,
                                                                                 &ThisClass::HandleTranslationsApplied);
    }
}

void UEVDBLanguageContextAndTranslationsWidget::NativeConstruct()
{
    Super::NativeConstruct();
    PopulateDatabaseContextComboBox();
    ApplyPreferencesToChildren();
}

void UEVDBLanguageContextAndTranslationsWidget::SetInitialPreferences(
    const FEVVocabularyLanguagePreferences& InPreferences)
{
    WorkingPreferences = InPreferences;
    WorkingPreferences.Normalize();
    PopulateDatabaseContextComboBox();
    ApplyPreferencesToChildren();
}

void UEVDBLanguageContextAndTranslationsWidget::ResetPendingChanges()
{
    if (WBP_VocabularyTranslationsWidget)
    {
        WBP_VocabularyTranslationsWidget->ResetPendingChanges();
    }
}

void UEVDBLanguageContextAndTranslationsWidget::PopulateDatabaseContextComboBox()
{
    if (!ComboBoxString_DB_Language_Context)
    {
        return;
    }

    bApplyingPreferences = true;
    ComboBoxString_DB_Language_Context->ClearOptions();

    for (const EEVVocabularyDBContext Context : EVVocabularyLanguage::GetAvailableDatabaseContexts())
    {
        ComboBoxString_DB_Language_Context->AddOption(
            EVVocabularyLanguage::GetDatabaseContextDisplayText(Context).ToString());
    }

    ComboBoxString_DB_Language_Context->SetSelectedOption(
        EVVocabularyLanguage::GetDatabaseContextDisplayText(WorkingPreferences.DatabaseContext).ToString());
    bApplyingPreferences = false;
}

void UEVDBLanguageContextAndTranslationsWidget::ApplyPreferencesToChildren()
{
    WorkingPreferences.Normalize();
    if (WBP_VocabularyTranslationsWidget)
    {
        WBP_VocabularyTranslationsWidget->SetInitialState(WorkingPreferences.DatabaseContext,
                                                          WorkingPreferences.SelectedTranslations);
    }
}

void UEVDBLanguageContextAndTranslationsWidget::HandleDatabaseContextSelected(const FString SelectedItem,
                                                                              const ESelectInfo::Type SelectionType)
{
    if (bApplyingPreferences)
    {
        return;
    }

    EEVVocabularyDBContext SelectedContext = EEVVocabularyDBContext::None;
    if (!EVVocabularyLanguage::TryParseDatabaseContext(SelectedItem, SelectedContext))
    {
        return;
    }

    WorkingPreferences.DatabaseContext = SelectedContext;
    WorkingPreferences.Normalize();

    if (WBP_VocabularyTranslationsWidget)
    {
        WBP_VocabularyTranslationsWidget->SetDatabaseContext(SelectedContext);
        WorkingPreferences.SelectedTranslations = WBP_VocabularyTranslationsWidget->GetWorkingTranslations();
        WorkingPreferences.Normalize();
    }

    // Context selection is committed immediately because it is the active DB context.
    OnPreferencesChanged.Broadcast(WorkingPreferences);
}

void UEVDBLanguageContextAndTranslationsWidget::HandleTranslationsApplied(
    const TArray<EEVVocabularyTranslationLanguage>& SelectedTranslations)
{
    WorkingPreferences.SelectedTranslations = SelectedTranslations;
    WorkingPreferences.Normalize();
    OnPreferencesChanged.Broadcast(WorkingPreferences);
}
