#pragma once

#include "CoreMinimal.h"
#include "EVConfirmationDialogActionTypes.generated.h"

UENUM(BlueprintType)
enum class EEVConfirmationDialogType : uint8
{
    Unknown UMETA(DisplayName = "Unknown"),
    ExitViewWord UMETA(DisplayName = "Exit the Detailed View mode?"),
    DeleteWord UMETA(DisplayName = "Are you sure you want to delete this word?")
};

USTRUCT(BlueprintType)
struct FEVConfirmationDialogInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EEVConfirmationDialogType DialogType = EEVConfirmationDialogType::Unknown;

    UPROPERTY(BlueprintReadOnly)
    FText DialogText;

    UPROPERTY(BlueprintReadOnly)
    FText ConfirmButtonText;

    UPROPERTY(BlueprintReadOnly)
    FText DiscardButtonText;

    UPROPERTY(BlueprintReadOnly)
    FLinearColor DialogTextColor = FLinearColor::White;

    UPROPERTY(BlueprintReadOnly)
    FLinearColor ConfirmButtonColor = FLinearColor::White;

    UPROPERTY(BlueprintReadOnly)
    FLinearColor DiscardButtonColor = FLinearColor::White;

    void Generate()
    {
        GenerateText();
        GenerateColors();
    }

private:
    void GenerateText()
    {
        DialogText = GetEnumDisplayName(DialogType);

        switch (DialogType)
        {
        case EEVConfirmationDialogType::DeleteWord:
            ConfirmButtonText = FText::FromString(TEXT("DELETE"));
            DiscardButtonText = FText::FromString(TEXT("CANCEL"));
            break;

        case EEVConfirmationDialogType::ExitViewWord:
            ConfirmButtonText = FText::FromString(TEXT("EXIT"));
            DiscardButtonText = FText::FromString(TEXT("STAY"));
            break;

        default:
            ConfirmButtonText = FText::FromString(TEXT("CONFIRM"));
            DiscardButtonText = FText::FromString(TEXT("DISCARD"));
            break;
        }
    }

    void GenerateColors()
    {
        switch (DialogType)
        {
        case EEVConfirmationDialogType::DeleteWord:
            DialogTextColor = FLinearColor(0.85f, 0.15f, 0.15f);
            ConfirmButtonColor = FLinearColor(0.85f, 0.15f, 0.15f);
            DiscardButtonColor = FLinearColor(0.15f, 0.75f, 0.20f);
            break;

        case EEVConfirmationDialogType::ExitViewWord:
            DialogTextColor = FLinearColor(0.85f, 0.15f, 0.15f);
            ConfirmButtonColor = FLinearColor(0.85f, 0.15f, 0.15f);
            DiscardButtonColor = FLinearColor(0.15f, 0.75f, 0.20f);
            break;

        default:
            DialogTextColor = FLinearColor::White;
            ConfirmButtonColor = FLinearColor::White;
            DiscardButtonColor = FLinearColor::White;
            break;
        }
    }

    static FText GetEnumDisplayName(EEVConfirmationDialogType Value)
    {
        if (const UEnum* Enum = StaticEnum<EEVConfirmationDialogType>())
        {
            return Enum->GetDisplayNameTextByValue(static_cast<int64>(Value));
        }

        return FText::GetEmpty();
    }
};