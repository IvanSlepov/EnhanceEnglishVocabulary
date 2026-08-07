#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVVocabularyFilterChipWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVocabularyFilterChipRemoveRequested, UEVVocabularyFilterChipWidget*,
                                            ChipWidget);

/**
 * Read-only removable chip used by the Vocabulary Filter dialog.
 *
 * Examples:
 * - Part of speech
 * - Noun
 * - Verb
 * - British
 */
UCLASS()
class ENHANCEVOCABULARYUI_API UEVVocabularyFilterChipWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Vocabulary Filter")
    void SetChipText(const FText& InText);

    UFUNCTION(BlueprintPure, Category = "Vocabulary Filter")
    FText GetChipText() const;

    UPROPERTY(BlueprintAssignable, Category = "Vocabulary Filter")
    FOnVocabularyFilterChipRemoveRequested OnRemoveRequested;

protected:
    virtual void NativeOnInitialized() override;

private:
    UFUNCTION()
    void HandleRemoveButtonPressed();

private:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_Value = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Remove = nullptr;
};