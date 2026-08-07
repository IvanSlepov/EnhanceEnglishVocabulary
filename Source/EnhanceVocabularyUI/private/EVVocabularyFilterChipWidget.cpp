#include "EVVocabularyFilterChipWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UEVVocabularyFilterChipWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_Remove)
    {
        Button_Remove->OnClicked.AddDynamic(this, &UEVVocabularyFilterChipWidget::HandleRemoveButtonPressed);
    }
}

void UEVVocabularyFilterChipWidget::SetChipText(const FText& InText)
{
    if (TextBlock_Value)
    {
        TextBlock_Value->SetText(InText);
    }
}

FText UEVVocabularyFilterChipWidget::GetChipText() const
{
    return TextBlock_Value ? TextBlock_Value->GetText() : FText::GetEmpty();
}

void UEVVocabularyFilterChipWidget::HandleRemoveButtonPressed()
{
    OnRemoveRequested.Broadcast(this);
}