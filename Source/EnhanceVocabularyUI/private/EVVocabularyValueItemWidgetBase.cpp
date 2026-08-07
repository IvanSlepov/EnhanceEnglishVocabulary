#include "EVVocabularyValueItemWidgetBase.h"

#include "Components/Button.h"
#include "Components/MultiLineEditableTextBox.h"
#include "EVVocabularyUiStyle.h"

void UEVVocabularyValueItemWidgetBase::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (!MultiLineEditableTextBox_Value)
    {
        UE_LOG(LogTemp, Error,
               TEXT("MultiLineEditableTextBox_Value is nullptr in "
                    "EVVocabularyValueItemWidgetBase."));

        return;
    }

    if (!Button_Delete)
    {
        UE_LOG(LogTemp, Error,
               TEXT("Button_Delete is nullptr in "
                    "EVVocabularyValueItemWidgetBase."));

        return;
    }

    OriginalTextBoxStyle = MultiLineEditableTextBox_Value->WidgetStyle;

    if (!Button_ValueAction)
    {
        UE_LOG(LogTemp, Error, TEXT("Button_ValueAction is nullptr in EVVocabularyValueItemWidgetBase."));
        return;
    }

    MultiLineEditableTextBox_Value->OnTextChanged.AddUniqueDynamic(this, &ThisClass::HandleValueTextChanged);

    MultiLineEditableTextBox_Value->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::HandleValueTextCommitted);

    Button_Delete->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleDeleteButtonClicked);
    Button_ValueAction->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleValueActionButtonPressed);

    MultiLineEditableTextBox_Value->SetText(FText::FromString(Value));
}

void UEVVocabularyValueItemWidgetBase::NativeConstruct()
{
    Super::NativeConstruct();

    ApplyEditableState();
}

void UEVVocabularyValueItemWidgetBase::NativeDestruct()
{
    if (MultiLineEditableTextBox_Value)
    {
        MultiLineEditableTextBox_Value->OnTextChanged.RemoveDynamic(this, &ThisClass::HandleValueTextChanged);

        MultiLineEditableTextBox_Value->OnTextCommitted.RemoveDynamic(this, &ThisClass::HandleValueTextCommitted);
    }

    if (Button_Delete)
    {
        Button_Delete->OnClicked.RemoveDynamic(this, &ThisClass::HandleDeleteButtonClicked);
    }

    if (Button_ValueAction)
    {
        Button_ValueAction->OnPressed.RemoveDynamic(this, &ThisClass::HandleValueActionButtonPressed);
    }

    Super::NativeDestruct();
}

void UEVVocabularyValueItemWidgetBase::SetValue(const FString& InValue)
{
    Value = InValue;

    if (!MultiLineEditableTextBox_Value)
    {
        return;
    }

    const FString CurrentDisplayedValue = MultiLineEditableTextBox_Value->GetText().ToString();

    if (!CurrentDisplayedValue.Equals(Value, ESearchCase::CaseSensitive))
    {
        MultiLineEditableTextBox_Value->SetText(FText::FromString(Value));
    }
}

const FString& UEVVocabularyValueItemWidgetBase::GetValue() const
{
    return Value;
}

void UEVVocabularyValueItemWidgetBase::SetEditable(const bool bInEditable)
{
    SetItemMode(bInEditable ? EEVVocabularyValueItemMode::DetailedEditable
                            : EEVVocabularyValueItemMode::DetailedReadOnly);
}

bool UEVVocabularyValueItemWidgetBase::IsEditable() const
{
    return bEditable;
}

void UEVVocabularyValueItemWidgetBase::SetItemMode(const EEVVocabularyValueItemMode InMode)
{
    ItemMode = InMode;
    bEditable = ItemMode == EEVVocabularyValueItemMode::DetailedEditable;
    ApplyEditableState();
}

EEVVocabularyValueItemMode UEVVocabularyValueItemWidgetBase::GetItemMode() const
{
    return ItemMode;
}

void UEVVocabularyValueItemWidgetBase::SetHintText(const FText& InHintText)
{
    if (MultiLineEditableTextBox_Value)
    {
        MultiLineEditableTextBox_Value->SetHintText(InHintText);
    }
}

void UEVVocabularyValueItemWidgetBase::SetPendingAddItem(const bool bInPendingAddItem)
{
    bPendingAddItem = bInPendingAddItem;
    ApplyEditableState();
}

bool UEVVocabularyValueItemWidgetBase::IsPendingAddItem() const
{
    return bPendingAddItem;
}

void UEVVocabularyValueItemWidgetBase::HandleValueTextChanged(const FText& NewText)
{
    Value = NewText.ToString();

    OnValueChanged.Broadcast(this, Value);
}

void UEVVocabularyValueItemWidgetBase::HandleValueTextCommitted(const FText& CommittedText,
                                                                const ETextCommit::Type CommitMethod)
{
    Value = CommittedText.ToString();

    OnValueCommitted.Broadcast(this, Value, CommitMethod);
}

void UEVVocabularyValueItemWidgetBase::HandleValueActionButtonPressed()
{
    if (ItemMode != EEVVocabularyValueItemMode::ReviewReadOnly || bPendingAddItem)
    {
        return;
    }

    OnValuePressed.Broadcast(this, Value);
}

void UEVVocabularyValueItemWidgetBase::HandleDeleteButtonClicked()
{
    if (!bEditable && ItemMode != EEVVocabularyValueItemMode::FilterRemovableReadOnly)
    {
        return;
    }

    OnDeleteRequested.Broadcast(this, Value);
}

void UEVVocabularyValueItemWidgetBase::ApplyEditableState()
{
    const bool bIsReview = ItemMode == EEVVocabularyValueItemMode::ReviewReadOnly;
    const bool bIsDetailedEditable = ItemMode == EEVVocabularyValueItemMode::DetailedEditable;
    const bool bIsFilterChip = ItemMode == EEVVocabularyValueItemMode::FilterRemovableReadOnly;

    if (MultiLineEditableTextBox_Value)
    {
        MultiLineEditableTextBox_Value->SetIsReadOnly(!bIsDetailedEditable);
        MultiLineEditableTextBox_Value->WidgetStyle.BackgroundImageReadOnly.TintColor =
            (bIsReview || bIsFilterChip) ? EVVocabularyUiStyle::GetReviewValueItemBackgroundTint()
                                         : OriginalTextBoxStyle.BackgroundImageReadOnly.TintColor;
        MultiLineEditableTextBox_Value->SynchronizeProperties();
    }

    if (Button_ValueAction)
    {
        Button_ValueAction->SetVisibility(bIsReview && !bPendingAddItem ? ESlateVisibility::Visible
                                                                        : ESlateVisibility::Collapsed);
    }

    if (Button_Delete)
    {
        Button_Delete->SetVisibility((bIsDetailedEditable || bIsFilterChip) && !bPendingAddItem
                                         ? ESlateVisibility::Visible
                                         : ESlateVisibility::Collapsed);
    }
}
