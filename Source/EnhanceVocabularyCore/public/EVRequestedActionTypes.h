#pragma once

#include "Math/Color.h"
#include "EVRequestedActionTypes.generated.h"

UENUM(BlueprintType)
enum class EEVRequestedActionSource : uint8
{
    Unknown,
    Database,
    AddWord,
    ReviewWords,
    ImportExport,
    Settings,
    ConnectionModule
};

UENUM(BlueprintType)
enum class EEVRequestedActionType : uint8
{
    Unknown,
    SaveWord,
    EditWord,
    DeleteWord,
    DownloadDBTemplate,
    ImportDBOverwrite,
    ImportDBAppend,
    ExportDB
};

UENUM(BlueprintType)
enum class EEVRequestedActionStatus : uint8
{
    Unknown UMETA(DisplayName = "Unknown"),
    InProgress UMETA(DisplayName = "In progress"),
    Done UMETA(DisplayName = "Done"),
    Saved UMETA(DisplayName = "Saved"),
    Completed UMETA(DisplayName = "Completed"),
    Warning UMETA(DisplayName = "Warning"),
    Failed UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FEVRequestedActionInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EEVRequestedActionSource Source = EEVRequestedActionSource::Unknown;

    UPROPERTY(BlueprintReadOnly)
    EEVRequestedActionType Type = EEVRequestedActionType::Unknown;

    UPROPERTY(BlueprintReadOnly)
    EEVRequestedActionStatus Status = EEVRequestedActionStatus::Unknown;

    UPROPERTY(BlueprintReadOnly)
    FText Message;

    UPROPERTY(BlueprintReadOnly)
    FLinearColor Color;

    void GenerateMessage()
    {
        if (const UEnum* Enum = StaticEnum<EEVRequestedActionStatus>())
        {
            Message = Enum->GetDisplayNameTextByValue(static_cast<int64>(Status));
        }
        else
        {
            Message = FText::GetEmpty();
        }
    }

    void GenerateColor()
    {
        switch (Status)
        {
        case EEVRequestedActionStatus::InProgress:
            Color = FLinearColor(0.10f, 0.25f, 0.75f); // Dark Blue
            break;

        // The Completed, Done and Saved fields effectively are the same
        // We may invoke different statuses just to obtain different text
        // for the specific action. For instance: 'Saved' may be set upon saving the word
        // while 'Done' or 'Completed' may be set upon importing/exporting etc
        // but the essence of each of these 3 is just to confirm that the requested action
        // has been successfully processed.
        case EEVRequestedActionStatus::Completed:
            Color = FLinearColor(0.15f, 0.75f, 0.20f); // Green
            break;

        case EEVRequestedActionStatus::Saved:
            Color = FLinearColor(0.15f, 0.75f, 0.20f); // Green
            break;

        case EEVRequestedActionStatus::Done:
            Color = FLinearColor(0.15f, 0.75f, 0.20f); // Green
            break;

        case EEVRequestedActionStatus::Failed:
            Color = FLinearColor(0.85f, 0.15f, 0.15f); // Red
            break;

        case EEVRequestedActionStatus::Warning:
            Color = FLinearColor(1.00f, 0.60f, 0.00f); // Orange
            break;

        default:
            Color = FLinearColor::White;
            break;
        }
    }
};