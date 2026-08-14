#include "EVVocabularyTranslationsWidget.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "EVVocabularyActiveTranslations.h"
#include "EVVocabularyTranslationOption.h"
#include "EVVocabularyUiStyle.h"

void UEVVocabularyTranslationsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_Close)
    {
        Button_Close->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleClosePressed);
    }
    if (Button_TranslationSelector)
    {
        Button_TranslationSelector->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleTranslationSelectorPressed);
    }
    if (Button_ApplyChanges)
    {
        Button_ApplyChanges->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleApplyChangesPressed);
    }
}

void UEVVocabularyTranslationsWidget::NativeConstruct()
{
    Super::NativeConstruct();
    bTranslationsExpanded = false;
    if (Border_TranslationsPanel)
    {
        Border_TranslationsPanel->SetVisibility(ESlateVisibility::Collapsed);
    }
    PopulateTranslationOptions();
    RebuildActiveTranslations();
}

void UEVVocabularyTranslationsWidget::SetInitialState(
    const EEVVocabularyDBContext InContext, const TArray<EEVVocabularyTranslationLanguage>& InSelectedTranslations)
{
    DatabaseContext = InContext;
    CommittedTranslations = InSelectedTranslations;
    WorkingTranslations = InSelectedTranslations;
    NormalizeWorkingTranslations();
    CommittedTranslations = WorkingTranslations;
    PopulateTranslationOptions();
    RebuildActiveTranslations();
}

void UEVVocabularyTranslationsWidget::ResetPendingChanges()
{
    WorkingTranslations = CommittedTranslations;
    NormalizeWorkingTranslations();
    SynchronizeOptionWidgets();
    RebuildActiveTranslations();

    bTranslationsExpanded = false;
    if (Border_TranslationsPanel)
    {
        Border_TranslationsPanel->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UEVVocabularyTranslationsWidget::SetDatabaseContext(const EEVVocabularyDBContext InContext)
{
    DatabaseContext = InContext;
    NormalizeWorkingTranslations();

    TArray<EEVVocabularyTranslationLanguage> NormalizedCommitted;
    for (const EEVVocabularyTranslationLanguage Language : CommittedTranslations)
    {
        if (Language != EEVVocabularyTranslationLanguage::None &&
            !EVVocabularyLanguage::IsTranslationEquivalentToDatabaseContext(Language, DatabaseContext) &&
            !NormalizedCommitted.Contains(Language))
        {
            NormalizedCommitted.Add(Language);
        }
    }
    CommittedTranslations = MoveTemp(NormalizedCommitted);

    PopulateTranslationOptions();
    RebuildActiveTranslations();
}

void UEVVocabularyTranslationsWidget::NormalizeWorkingTranslations()
{
    TArray<EEVVocabularyTranslationLanguage> Normalized;
    for (const EEVVocabularyTranslationLanguage Language : WorkingTranslations)
    {
        if (Language == EEVVocabularyTranslationLanguage::None ||
            EVVocabularyLanguage::IsTranslationEquivalentToDatabaseContext(Language, DatabaseContext) ||
            Normalized.Contains(Language))
        {
            continue;
        }
        Normalized.Add(Language);
    }
    WorkingTranslations = MoveTemp(Normalized);
}

void UEVVocabularyTranslationsWidget::PopulateTranslationOptions()
{
    if (!VerticalBox_Translations)
    {
        return;
    }

    VerticalBox_Translations->ClearChildren();
    TranslationOptionWidgets.Reset();

    if (!TranslationOptionWidgetClass)
    {
        UE_LOG(LogTemp, Error,
               TEXT("TranslationOptionWidgetClass is not assigned in WBP_VocabularyTranslationsWidget."));
        return;
    }

    for (const EEVVocabularyTranslationLanguage Language : EVVocabularyLanguage::GetAvailableTranslationLanguages())
    {
        if (EVVocabularyLanguage::IsTranslationEquivalentToDatabaseContext(Language, DatabaseContext))
        {
            continue;
        }

        UEVVocabularyTranslationOption* OptionWidget =
            CreateWidget<UEVVocabularyTranslationOption>(GetOwningPlayer(), TranslationOptionWidgetClass);
        if (!OptionWidget)
        {
            continue;
        }

        OptionWidget->ConfigureOption(Language, WorkingTranslations.Contains(Language));
        OptionWidget->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::HandleTranslationOptionSelectionChanged);

        if (UVerticalBoxSlot* OptionVerticalBoxSlot = VerticalBox_Translations->AddChildToVerticalBox(OptionWidget))
        {
            OptionVerticalBoxSlot->SetHorizontalAlignment(HAlign_Fill);
            OptionVerticalBoxSlot->SetVerticalAlignment(VAlign_Top);
            OptionVerticalBoxSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));
        }

        TranslationOptionWidgets.Add(Language, OptionWidget);
    }
}

void UEVVocabularyTranslationsWidget::RebuildActiveTranslations()
{
    if (!VerticalBox_ActiveTranslations)
    {
        UpdateEmptyState();
        return;
    }

    VerticalBox_ActiveTranslations->ClearChildren();

    if (!WorkingTranslations.IsEmpty() && ActiveTranslationsWidgetClass)
    {
        UEVVocabularyActiveTranslations* ActiveTranslations =
            CreateWidget<UEVVocabularyActiveTranslations>(GetOwningPlayer(), ActiveTranslationsWidgetClass);
        if (ActiveTranslations)
        {
            ActiveTranslations->SetTranslations(WorkingTranslations);
            ActiveTranslations->OnTranslationRemoveRequested.AddUniqueDynamic(
                this, &ThisClass::HandleActiveTranslationRemoveRequested);

            if (UVerticalBoxSlot* ActiveTranslationsSlot =
                    VerticalBox_ActiveTranslations->AddChildToVerticalBox(ActiveTranslations))
            {
                ActiveTranslationsSlot->SetHorizontalAlignment(HAlign_Fill);
                ActiveTranslationsSlot->SetVerticalAlignment(VAlign_Top);
            }
        }
    }

    UpdateEmptyState();
}

void UEVVocabularyTranslationsWidget::SynchronizeOptionWidgets()
{
    for (const TPair<EEVVocabularyTranslationLanguage, UEVVocabularyTranslationOption*>& Pair :
         TranslationOptionWidgets)
    {
        if (Pair.Value)
        {
            Pair.Value->SetSelected(WorkingTranslations.Contains(Pair.Key), false);
        }
    }
}

void UEVVocabularyTranslationsWidget::UpdateEmptyState()
{
    const bool bHasTranslations = !WorkingTranslations.IsEmpty();

    if (TextBlock_NoActiveTranslations)
    {
        TextBlock_NoActiveTranslations->SetText(
            bHasTranslations ? NSLOCTEXT("EVVocabularyTranslations", "ActiveTranslations", "ACTIVE TRANSLATIONS")
                             : NSLOCTEXT("EVVocabularyTranslations", "NoActiveTranslations", "NO ACTIVE TRANSLATIONS"));
        TextBlock_NoActiveTranslations->SetColorAndOpacity(
            bHasTranslations ? EVVocabularyUiStyle::GetActiveTranslationsTextFontColor()
                             : EVVocabularyUiStyle::GetInactiveTranslationsTextFontColor());
        TextBlock_NoActiveTranslations->SetVisibility(ESlateVisibility::Visible);
    }

    if (VerticalBox_ActiveTranslations)
    {
        VerticalBox_ActiveTranslations->SetVisibility(bHasTranslations ? ESlateVisibility::Visible
                                                                       : ESlateVisibility::Collapsed);
    }
}

void UEVVocabularyTranslationsWidget::HandleClosePressed()
{
    // Close acts as cancel for uncommitted translation changes while keeping the embedded settings section available.
    ResetPendingChanges();
}

void UEVVocabularyTranslationsWidget::HandleTranslationSelectorPressed()
{
    bTranslationsExpanded = !bTranslationsExpanded;
    if (Border_TranslationsPanel)
    {
        Border_TranslationsPanel->SetVisibility(bTranslationsExpanded ? ESlateVisibility::Visible
                                                                      : ESlateVisibility::Collapsed);
    }
}

void UEVVocabularyTranslationsWidget::HandleApplyChangesPressed()
{
    NormalizeWorkingTranslations();
    CommittedTranslations = WorkingTranslations;
    OnTranslationsApplied.Broadcast(CommittedTranslations);
    RebuildActiveTranslations();

    bTranslationsExpanded = false;
    if (Border_TranslationsPanel)
    {
        Border_TranslationsPanel->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UEVVocabularyTranslationsWidget::HandleTranslationOptionSelectionChanged(
    const EEVVocabularyTranslationLanguage Language, const bool bSelected)
{
    if (bSelected)
    {
        if (!EVVocabularyLanguage::IsTranslationEquivalentToDatabaseContext(Language, DatabaseContext) &&
            !WorkingTranslations.Contains(Language))
        {
            WorkingTranslations.Add(Language);
        }
    }
    else
    {
        WorkingTranslations.Remove(Language);
    }

    NormalizeWorkingTranslations();
    RebuildActiveTranslations();
}

void UEVVocabularyTranslationsWidget::HandleActiveTranslationRemoveRequested(
    const EEVVocabularyTranslationLanguage Language)
{
    WorkingTranslations.Remove(Language);
    NormalizeWorkingTranslations();
    SynchronizeOptionWidgets();
    RebuildActiveTranslations();
}
