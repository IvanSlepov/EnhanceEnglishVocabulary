#pragma once

#include "CoreMinimal.h"
#include "EVConfirmationDialogActionTypes.generated.h"

UENUM(BlueprintType)
enum class EEVConfirmationDialogType : uint8
{
    Unknown UMETA(DisplayName = "Unknown"),
    ExitViewWord UMETA(DisplayName = "Exit the Detailed View mode?"),
    EditWord UMETA(DisplayName = "Overwrite the existing word's fields?"),
    DeleteWord UMETA(DisplayName = "Are you sure you want to delete this word?"),
    OverwriteDB UMETA(DisplayName = "Warning! You are about to overwrite your current DataBase. We strongly recommend "
                                    "to EXPORT it before proceeding!"),
    AppendDB UMETA(DisplayName = "Append imported data to the current DataBase?")
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
        switch (DialogType)
        {
        case EEVConfirmationDialogType::DeleteWord:
            DialogText = FText::FromString(TEXT("Are you sure you want to delete this word?"));
            ConfirmButtonText = FText::FromString(TEXT("DELETE"));
            DiscardButtonText = FText::FromString(TEXT("CANCEL"));
            break;

        case EEVConfirmationDialogType::ExitViewWord:
            DialogText = FText::FromString(TEXT("Exit the Detailed View mode?"));
            ConfirmButtonText = FText::FromString(TEXT("EXIT"));
            DiscardButtonText = FText::FromString(TEXT("STAY"));
            break;

        case EEVConfirmationDialogType::EditWord:
            DialogText = FText::FromString(TEXT("Overwrite the existing word's fields?"));
            ConfirmButtonText = FText::FromString(TEXT("OVERWRITE"));
            DiscardButtonText = FText::FromString(TEXT("RETAIN"));
            break;

        case EEVConfirmationDialogType::OverwriteDB:
            DialogText = FText::FromString(TEXT("Warning! You are about to overwrite your current DataBase. We "
                                                "strongly recommend to EXPORT it before proceeding!"));
            ConfirmButtonText = FText::FromString(TEXT("OVERWRITE"));
            DiscardButtonText = FText::FromString(TEXT("CANCEL"));
            break;

        case EEVConfirmationDialogType::AppendDB:
            DialogText = FText::FromString(
                TEXT("Append imported data to your current DataBase? Existing entries will be retained."));
            ConfirmButtonText = FText::FromString(TEXT("APPEND"));
            DiscardButtonText = FText::FromString(TEXT("CANCEL"));
            break;

        default:
            DialogText = FText::FromString(TEXT("Unknown"));
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

        case EEVConfirmationDialogType::EditWord:
            DialogTextColor = FLinearColor(0.85f, 0.15f, 0.15f);
            ConfirmButtonColor = FLinearColor(0.85f, 0.15f, 0.15f);
            DiscardButtonColor = FLinearColor(0.15f, 0.75f, 0.20f);
            break;

        case EEVConfirmationDialogType::OverwriteDB:
            DialogTextColor = FLinearColor(0.85f, 0.15f, 0.15f);
            ConfirmButtonColor = FLinearColor(0.85f, 0.15f, 0.15f);
            DiscardButtonColor = FLinearColor(0.15f, 0.75f, 0.20f);
            break;

        case EEVConfirmationDialogType::AppendDB:
            DialogTextColor = FLinearColor(1.00f, 0.60f, 0.00f);
            ConfirmButtonColor = FLinearColor(1.00f, 0.60f, 0.00f);
            DiscardButtonColor = FLinearColor(0.15f, 0.75f, 0.20f);
            break;

        default:
            DialogTextColor = FLinearColor::Black;
            ConfirmButtonColor = FLinearColor::Black;
            DiscardButtonColor = FLinearColor::Black;
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