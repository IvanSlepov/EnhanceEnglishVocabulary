#include "EVVocabularyValueItemWidgetBase.h"

#include "Components/Button.h"
#include "Components/MultiLineEditableTextBox.h"

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

    MultiLineEditableTextBox_Value->OnTextChanged.AddUniqueDynamic(this, &ThisClass::HandleValueTextChanged);

    MultiLineEditableTextBox_Value->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::HandleValueTextCommitted);

    Button_Delete->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleDeleteButtonClicked);

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
    bEditable = bInEditable;

    ApplyEditableState();
}

bool UEVVocabularyValueItemWidgetBase::IsEditable() const
{
    return bEditable;
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

void UEVVocabularyValueItemWidgetBase::HandleDeleteButtonClicked()
{
    if (!bEditable)
    {
        return;
    }

    OnDeleteRequested.Broadcast(this, Value);
}

void UEVVocabularyValueItemWidgetBase::ApplyEditableState()
{
    if (MultiLineEditableTextBox_Value)
    {
        MultiLineEditableTextBox_Value->SetIsReadOnly(!bEditable);
    }

    if (Button_Delete)
    {
        Button_Delete->SetVisibility(bEditable && !bPendingAddItem ? ESlateVisibility::Visible
                                                                   : ESlateVisibility::Collapsed);
    }
}