#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVCheckBoxWidget.generated.h"

class UButton;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEVCheckBoxStateChanged, bool, bIsChecked);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVCheckBoxWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "EV Check Box")
    void SetChecked(bool bInChecked, bool bBroadcastChange = false);

    UFUNCTION(BlueprintCallable, Category = "EV Check Box")
    void ToggleChecked();

    UFUNCTION(BlueprintPure, Category = "EV Check Box")
    bool IsChecked() const
    {
        return bIsChecked;
    }

    UPROPERTY(BlueprintAssignable, Category = "EV Check Box|Events")
    FOnEVCheckBoxStateChanged OnCheckStateChanged;

protected:
    virtual void NativeOnInitialized() override;

private:
    UFUNCTION()
    void HandleButtonPressed();

    void ApplyVisualState();

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> Image_Tick = nullptr;

    UPROPERTY(Transient)
    bool bIsChecked = false;
};
