#pragma once

#include "CoreMinimal.h"
#include "EVPopUpSettingsTypes.generated.h"

UENUM(BlueprintType)
enum class EEVPopUpIntervals : uint8
{
    TurnedOff UMETA(DisplayName = "Turned Off"),
    One_Minute UMETA(DisplayName = "1 minute"),
    Two_Minutes UMETA(DisplayName = "2 minutes"),
    Five_Minutes UMETA(DisplayName = "5 minutes"),
    Ten_Minutes UMETA(DisplayName = "10 minutes"),
    Fifteen_Minutes UMETA(DisplayName = "15 minutes"),
    Twenty_Minutes UMETA(DisplayName = "20 minutes"),
    TwentyFive_Minutes UMETA(DisplayName = "25 minutes"),
    Thirty_Minutes UMETA(DisplayName = "30 minutes"),
    ThirtyFive_Minutes UMETA(DisplayName = "35 minutes"),
    Forty_Minutes UMETA(DisplayName = "40 minutes"),
    FortyFive_Minutes UMETA(DisplayName = "45 minutes"),
    Fifty_Minutes UMETA(DisplayName = "50 minutes"),
    FiftyFive_Minutes UMETA(DisplayName = "55 minutes"),
    Sixty_Minutes UMETA(DisplayName = "60 minutes")
};

UENUM(BlueprintType)
enum class EEVNotificationMode : uint8
{
    RandomWord UMETA(DisplayName = "Random Word"),
    TestMode UMETA(DisplayName = "Test Mode") // -- debugging mode
    // More modes later...
};

USTRUCT(BlueprintType)
struct FEVPopUpSettingsInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pop-Up")
    EEVPopUpIntervals PopUpIntervals = EEVPopUpIntervals::TurnedOff;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pop-Up")
    EEVNotificationMode NotificationMode = EEVNotificationMode::RandomWord;

    /**
     * Returns all visible EEVPopUpIntervals enum DisplayNames.
     */
    static FText GetIntervalDisplayName(const EEVPopUpIntervals Interval)
    {
        switch (Interval)
        {
        case EEVPopUpIntervals::TurnedOff:
            return NSLOCTEXT("EVPopUpIntervals", "TurnedOff", "Turned Off");

        case EEVPopUpIntervals::One_Minute:
            return NSLOCTEXT("EVPopUpIntervals", "OneMinute", "1 minute");

        case EEVPopUpIntervals::Two_Minutes:
            return NSLOCTEXT("EVPopUpIntervals", "TwoMinutes", "2 minutes");

        case EEVPopUpIntervals::Five_Minutes:
            return NSLOCTEXT("EVPopUpIntervals", "FiveMinutes", "5 minutes");

        case EEVPopUpIntervals::Ten_Minutes:
            return NSLOCTEXT("EVPopUpIntervals", "TenMinutes", "10 minutes");

        case EEVPopUpIntervals::Fifteen_Minutes:
            return NSLOCTEXT("EVPopUpIntervals", "FifteenMinutes", "15 minutes");

        case EEVPopUpIntervals::Twenty_Minutes:
            return NSLOCTEXT("EVPopUpIntervals", "TwentyMinutes", "20 minutes");

        case EEVPopUpIntervals::TwentyFive_Minutes:
            return NSLOCTEXT("EVPopUpIntervals", "TwentyFiveMinutes", "25 minutes");

        case EEVPopUpIntervals::Thirty_Minutes:
            return NSLOCTEXT("EVPopUpIntervals", "ThirtyMinutes", "30 minutes");

        case EEVPopUpIntervals::ThirtyFive_Minutes:
            return NSLOCTEXT("EVPopUpIntervals", "ThirtyFiveMinutes", "35 minutes");

        case EEVPopUpIntervals::Forty_Minutes:
            return NSLOCTEXT("EVPopUpIntervals", "FortyMinutes", "40 minutes");

        case EEVPopUpIntervals::FortyFive_Minutes:
            return NSLOCTEXT("EVPopUpIntervals", "FortyFiveMinutes", "45 minutes");

        case EEVPopUpIntervals::Fifty_Minutes:
            return NSLOCTEXT("EVPopUpIntervals", "FiftyMinutes", "50 minutes");

        case EEVPopUpIntervals::FiftyFive_Minutes:
            return NSLOCTEXT("EVPopUpIntervals", "FiftyFiveMinutes", "55 minutes");

        case EEVPopUpIntervals::Sixty_Minutes:
            return NSLOCTEXT("EVPopUpIntervals", "SixtyMinutes", "60 minutes");

        default:
            ensureMsgf(false, TEXT("Invalid pop-up interval value."));
            return FText::GetEmpty();
        }
    }

    static TArray<EEVPopUpIntervals> GetAllIntervals()
    {
        return {EEVPopUpIntervals::TurnedOff,      EEVPopUpIntervals::One_Minute,
                EEVPopUpIntervals::Two_Minutes,    EEVPopUpIntervals::Five_Minutes,
                EEVPopUpIntervals::Ten_Minutes,    EEVPopUpIntervals::Fifteen_Minutes,
                EEVPopUpIntervals::Twenty_Minutes, EEVPopUpIntervals::TwentyFive_Minutes,
                EEVPopUpIntervals::Thirty_Minutes, EEVPopUpIntervals::ThirtyFive_Minutes,
                EEVPopUpIntervals::Forty_Minutes,  EEVPopUpIntervals::FortyFive_Minutes,
                EEVPopUpIntervals::Fifty_Minutes,  EEVPopUpIntervals::FiftyFive_Minutes,
                EEVPopUpIntervals::Sixty_Minutes};
    }

    static TArray<FText> GetAllIntervalDisplayNames()
    {
        TArray<FText> Result;

        for (const EEVPopUpIntervals Interval : GetAllIntervals())
        {
            Result.Add(GetIntervalDisplayName(Interval));
        }

        return Result;
    }

    /**
     * Finds an enum value by its DisplayName.
     *
     * Returns true when a matching entry is found.
     */
    static bool TryGetIntervalFromDisplayName(const FString& DisplayName, EEVPopUpIntervals& OutInterval)
    {
        for (const EEVPopUpIntervals Interval : GetAllIntervals())
        {
            if (GetIntervalDisplayName(Interval).ToString().Equals(DisplayName, ESearchCase::IgnoreCase))
            {
                OutInterval = Interval;
                return true;
            }
        }

        return false;
    }

    /**
     * Return a specific int value representing the time interval in seconds.
     */
    static int32 GetIntervalSeconds(const EEVPopUpIntervals Interval)
    {
        switch (Interval)
        {
        case EEVPopUpIntervals::TurnedOff:
            return 0;
        case EEVPopUpIntervals::One_Minute:
            return 1 * 60;
        case EEVPopUpIntervals::Two_Minutes:
            return 2 * 60;
        case EEVPopUpIntervals::Five_Minutes:
            return 5 * 60;
        case EEVPopUpIntervals::Ten_Minutes:
            return 10 * 60;
        case EEVPopUpIntervals::Fifteen_Minutes:
            return 15 * 60;
        case EEVPopUpIntervals::Twenty_Minutes:
            return 20 * 60;
        case EEVPopUpIntervals::TwentyFive_Minutes:
            return 25 * 60;
        case EEVPopUpIntervals::Thirty_Minutes:
            return 30 * 60;
        case EEVPopUpIntervals::ThirtyFive_Minutes:
            return 35 * 60;
        case EEVPopUpIntervals::Forty_Minutes:
            return 40 * 60;
        case EEVPopUpIntervals::FortyFive_Minutes:
            return 45 * 60;
        case EEVPopUpIntervals::Fifty_Minutes:
            return 50 * 60;
        case EEVPopUpIntervals::FiftyFive_Minutes:
            return 55 * 60;
        case EEVPopUpIntervals::Sixty_Minutes:
            return 60 * 60;
        default:
            ensureMsgf(false, TEXT("Invalid pop-up interval value."));
            return 0;
        }
    }

    /**
     * Returns all visible EEVNotificationMode enum DisplayNames.
     */
    static FText GetNotificationModesDisplayName(const EEVNotificationMode NotificationMode)
    {
        switch (NotificationMode)
        {
        case EEVNotificationMode::RandomWord:
            return NSLOCTEXT("EEVNotificationMode", "RandomWord", "Random Word");
        case EEVNotificationMode::TestMode:
            return NSLOCTEXT("EEVNotificationMode", "TestMode", "Test Mode");
        default:
            ensureMsgf(false, TEXT("Invalid notification mode value."));
            return FText::GetEmpty();
        }
    }

    static TArray<EEVNotificationMode> GetAllNotificationModes()
    {
        return {EEVNotificationMode::RandomWord, EEVNotificationMode::TestMode};
    }

    static TArray<FText> GetAllNotificationModesDisplayNames()
    {
        TArray<FText> Result;

        for (const EEVNotificationMode NotificationMode : GetAllNotificationModes())
        {
            Result.Add(GetNotificationModesDisplayName(NotificationMode));
        }

        return Result;
    }

    /**
     * Finds an enum value by its DisplayName.
     *
     * Returns true when a matching entry is found.
     */
    static bool TryGetNotificationModeFromDisplayName(const FString& DisplayName,
                                                      EEVNotificationMode& OutNotificationMode)
    {
        for (const EEVNotificationMode NotificationMode : GetAllNotificationModes())
        {
            if (GetNotificationModesDisplayName(NotificationMode)
                    .ToString()
                    .Equals(DisplayName, ESearchCase::IgnoreCase))
            {
                OutNotificationMode = NotificationMode;
                return true;
            }
        }

        return false;
    }
};