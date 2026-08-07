#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateTypes.h"
#include "Blueprint/UserWidget.h"
#include "EVVocabularyInteractionTypes.h"
#include "EVVocabularyValueItemWidgetBase.generated.h"

class UButton;
class UMultiLineEditableTextBox;
class UEVVocabularyValueItemWidgetBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVocabularyValueItemChanged, UEVVocabularyValueItemWidgetBase*,
                                             ItemWidget, const FString&, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVocabularyValueItemCommitted, UEVVocabularyValueItemWidgetBase*,
                                               ItemWidget, const FString&, CommittedValue, ETextCommit::Type,
                                               CommitMethod);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVocabularyValueItemDeleteRequested, UEVVocabularyValueItemWidgetBase*,
                                             ItemWidget, const FString&, Value);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVocabularyValueItemPressed, UEVVocabularyValueItemWidgetBase*,
                                             ItemWidget, const FString&, Value);

/**
 * Shared base class for compact vocabulary value widgets.
 *
 * Used as the parent class for:
 * - Translation item widgets
 * - Synonym item widgets
 * - Antonym item widgets
 *
 * The widget owns:
 * - the displayed/editable value;
 * - read-only/editable state;
 * - delete-button state;
 * - change, commit and delete-request delegates.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class ENHANCEVOCABULARYUI_API UEVVocabularyValueItemWidgetBase : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     * Sets the text displayed by this item.
     */
    UFUNCTION(BlueprintCallable, Category = "Vocabulary Value Item")
    void SetValue(const FString& InValue);

    /**
     * Returns the current value.
     */
    UFUNCTION(BlueprintPure, Category = "Vocabulary Value Item")
    const FString& GetValue() const;

    /**
     * Enables or disables editing.
     *
     * Read-only mode:
     * - text cannot be edited;
     * - delete button is collapsed.
     *
     * Editable mode:
     * - text can be edited;
     * - delete button is visible.
     */
    UFUNCTION(BlueprintCallable, Category = "Vocabulary Value Item")
    void SetEditable(bool bInEditable);

    /**
     * Returns whether the value is currently editable.
     */
    UFUNCTION(BlueprintPure, Category = "Vocabulary Value Item")
    bool IsEditable() const;

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Value Item")
    void SetItemMode(EEVVocabularyValueItemMode InMode);

    UFUNCTION(BlueprintPure, Category = "Vocabulary Value Item")
    EEVVocabularyValueItemMode GetItemMode() const;

    /** Sets the input hint shown when this item is empty. */
    UFUNCTION(BlueprintCallable, Category = "Vocabulary Value Item")
    void SetHintText(const FText& InHintText);

    /** Marks this widget as the dedicated new-value input slot. */
    UFUNCTION(BlueprintCallable, Category = "Vocabulary Value Item")
    void SetPendingAddItem(bool bInPendingAddItem);

    UFUNCTION(BlueprintPure, Category = "Vocabulary Value Item")
    bool IsPendingAddItem() const;

    /**
     * Fired whenever the text changes.
     */
    UPROPERTY(BlueprintAssignable, Category = "Vocabulary Value Item|Events")
    FOnVocabularyValueItemChanged OnValueChanged;

    /**
     * Fired when the user commits the edited value.
     */
    UPROPERTY(BlueprintAssignable, Category = "Vocabulary Value Item|Events")
    FOnVocabularyValueItemCommitted OnValueCommitted;

    /**
     * Fired when the delete button is pressed.
     *
     * This widget does not remove itself.
     * The parent widget owns removal from its data collection and container.
     */
    UPROPERTY(BlueprintAssignable, Category = "Vocabulary Value Item|Events")
    FOnVocabularyValueItemDeleteRequested OnDeleteRequested;

    UPROPERTY(BlueprintAssignable, Category = "Vocabulary Value Item|Events")
    FOnVocabularyValueItemPressed OnValuePressed;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    UFUNCTION()
    void HandleValueTextChanged(const FText& NewText);

    UFUNCTION()
    void HandleValueTextCommitted(const FText& CommittedText, ETextCommit::Type CommitMethod);

    UFUNCTION()
    void HandleDeleteButtonClicked();

    UFUNCTION()
    void HandleValueActionButtonPressed();

    void ApplyEditableState();

private:
    /**
     * Must be named exactly:
     * MultiLineEditableTextBox_Value
     */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UMultiLineEditableTextBox> MultiLineEditableTextBox_Value = nullptr;

    /**
     * Must be named exactly:
     * Button_Delete
     */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Delete = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_ValueAction = nullptr;

    UPROPERTY(Transient)
    FString Value;

    UPROPERTY(Transient)
    bool bEditable = false;

    UPROPERTY(Transient)
    bool bPendingAddItem = false;

    UPROPERTY(Transient)
    EEVVocabularyValueItemMode ItemMode = EEVVocabularyValueItemMode::DetailedReadOnly;

    FEditableTextBoxStyle OriginalTextBoxStyle;
};