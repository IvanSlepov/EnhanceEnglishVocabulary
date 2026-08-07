#include "EVCheckBoxWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"

void UEVCheckBoxWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button)
    {
        Button->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleButtonPressed);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Button is nullptr in EVCheckBoxWidget."));
    }

    if (!Image_Tick)
    {
        UE_LOG(LogTemp, Error, TEXT("Image_Tick is nullptr in EVCheckBoxWidget."));
    }

    ApplyVisualState();
}

void UEVCheckBoxWidget::SetChecked(const bool bInChecked, const bool bBroadcastChange)
{
    const bool bChanged = bIsChecked != bInChecked;
    bIsChecked = bInChecked;
    ApplyVisualState();

    if (bChanged && bBroadcastChange)
    {
        OnCheckStateChanged.Broadcast(bIsChecked);
    }
}

void UEVCheckBoxWidget::ToggleChecked()
{
    SetChecked(!bIsChecked, true);
}

void UEVCheckBoxWidget::HandleButtonPressed()
{
    ToggleChecked();
}

void UEVCheckBoxWidget::ApplyVisualState()
{
    if (Image_Tick)
    {
        Image_Tick->SetVisibility(bIsChecked ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}
