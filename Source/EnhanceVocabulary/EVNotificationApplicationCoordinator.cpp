#include "EVNotificationApplicationCoordinator.h"

#include "EVDeviceService.h"
#include "EVVocabularyStorageService.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
struct FEVRandomWordNotificationSelection
{
    FString Word;
    FString NormalizedWord;
    FString Transcription;
    FString PartOfSpeech;
    int32 MeaningDisplayOrder = 0;
    FString DefinitionText;
    int32 DefinitionDisplayOrder = 0;
};

FString LastRandomNotificationNormalizedWord;
int32 LastRandomNotificationMeaningDisplayOrder = INDEX_NONE;
int32 LastRandomNotificationDefinitionDisplayOrder = INDEX_NONE;

FString ResolveNotificationTranscription(const FEVVocabularyRecord& Record)
{
    static const FString VocabularyLanguageCode = TEXT("en");

    for (const FEVVocabularyPronunciation& Pronunciation : Record.Pronunciations)
    {
        if (Pronunciation.LanguageCode.Equals(VocabularyLanguageCode, ESearchCase::IgnoreCase) &&
            Pronunciation.bPrimary && !Pronunciation.Transcription.TrimStartAndEnd().IsEmpty())
        {
            return Pronunciation.Transcription.TrimStartAndEnd();
        }
    }

    for (const FEVVocabularyPronunciation& Pronunciation : Record.Pronunciations)
    {
        if (Pronunciation.LanguageCode.Equals(VocabularyLanguageCode, ESearchCase::IgnoreCase) &&
            !Pronunciation.Transcription.TrimStartAndEnd().IsEmpty())
        {
            return Pronunciation.Transcription.TrimStartAndEnd();
        }
    }

    return TEXT("No transcription was provided");
}

bool HasUsableDefinitions(const FEVVocabularyMeaning& Meaning)
{
    return Meaning.Definitions.ContainsByPredicate([](const FEVVocabularyDefinition& Definition)
                                                   { return !Definition.DefinitionText.TrimStartAndEnd().IsEmpty(); });
}

bool SelectRandomWordNotification(const TArray<FEVVocabularyRecord>& Records,
                                  FEVRandomWordNotificationSelection& OutSelection)
{
    TArray<const FEVVocabularyRecord*> ValidRecords;

    for (const FEVVocabularyRecord& Record : Records)
    {
        if (Record.Word.TrimStartAndEnd().IsEmpty())
        {
            continue;
        }

        if (Record.Meanings.ContainsByPredicate([](const FEVVocabularyMeaning& Meaning)
                                                { return HasUsableDefinitions(Meaning); }))
        {
            ValidRecords.Add(&Record);
        }
    }

    if (ValidRecords.IsEmpty())
    {
        return false;
    }

    TArray<const FEVVocabularyRecord*> RecordCandidates = ValidRecords;

    if (RecordCandidates.Num() > 1 && !LastRandomNotificationNormalizedWord.IsEmpty())
    {
        RecordCandidates.RemoveAll(
            [](const FEVVocabularyRecord* Record) {
                return Record &&
                       Record->NormalizedWord.Equals(LastRandomNotificationNormalizedWord, ESearchCase::IgnoreCase);
            });
    }

    const FEVVocabularyRecord& SelectedRecord = *RecordCandidates[FMath::RandRange(0, RecordCandidates.Num() - 1)];

    TArray<const FEVVocabularyMeaning*> ValidMeanings;

    for (const FEVVocabularyMeaning& Meaning : SelectedRecord.Meanings)
    {
        if (HasUsableDefinitions(Meaning))
        {
            ValidMeanings.Add(&Meaning);
        }
    }

    TArray<const FEVVocabularyMeaning*> MeaningCandidates = ValidMeanings;

    const bool bSelectedSameWord =
        SelectedRecord.NormalizedWord.Equals(LastRandomNotificationNormalizedWord, ESearchCase::IgnoreCase);

    if (bSelectedSameWord && MeaningCandidates.Num() > 1)
    {
        MeaningCandidates.RemoveAll(
            [](const FEVVocabularyMeaning* Meaning)
            { return Meaning && Meaning->DisplayOrder == LastRandomNotificationMeaningDisplayOrder; });
    }

    const FEVVocabularyMeaning& SelectedMeaning = *MeaningCandidates[FMath::RandRange(0, MeaningCandidates.Num() - 1)];

    TArray<const FEVVocabularyDefinition*> ValidDefinitions;

    for (const FEVVocabularyDefinition& Definition : SelectedMeaning.Definitions)
    {
        if (!Definition.DefinitionText.TrimStartAndEnd().IsEmpty())
        {
            ValidDefinitions.Add(&Definition);
        }
    }

    TArray<const FEVVocabularyDefinition*> DefinitionCandidates = ValidDefinitions;

    const bool bSelectedSameMeaning =
        bSelectedSameWord && SelectedMeaning.DisplayOrder == LastRandomNotificationMeaningDisplayOrder;

    if (bSelectedSameMeaning && DefinitionCandidates.Num() > 1)
    {
        DefinitionCandidates.RemoveAll(
            [](const FEVVocabularyDefinition* Definition)
            { return Definition && Definition->DisplayOrder == LastRandomNotificationDefinitionDisplayOrder; });
    }

    const FEVVocabularyDefinition& SelectedDefinition =
        *DefinitionCandidates[FMath::RandRange(0, DefinitionCandidates.Num() - 1)];

    OutSelection.Word = SelectedRecord.Word.TrimStartAndEnd();
    OutSelection.NormalizedWord = SelectedRecord.NormalizedWord.TrimStartAndEnd();
    OutSelection.Transcription = ResolveNotificationTranscription(SelectedRecord);
    OutSelection.PartOfSpeech = SelectedMeaning.PartOfSpeech.TrimStartAndEnd();
    OutSelection.MeaningDisplayOrder = SelectedMeaning.DisplayOrder;
    OutSelection.DefinitionText = SelectedDefinition.DefinitionText.TrimStartAndEnd();
    OutSelection.DefinitionDisplayOrder = SelectedDefinition.DisplayOrder;

    if (OutSelection.PartOfSpeech.IsEmpty())
    {
        OutSelection.PartOfSpeech = TEXT("unspecified");
    }

    LastRandomNotificationNormalizedWord = OutSelection.NormalizedWord;
    LastRandomNotificationMeaningDisplayOrder = OutSelection.MeaningDisplayOrder;
    LastRandomNotificationDefinitionDisplayOrder = OutSelection.DefinitionDisplayOrder;

    return true;
}

bool BuildRandomWordNotificationPayload(const TArray<FEVVocabularyRecord>& Records, FString& OutPayload)
{
    OutPayload.Reset();

    TArray<TSharedPtr<FJsonValue>> RecordValues;

    for (const FEVVocabularyRecord& Record : Records)
    {
        if (Record.Word.TrimStartAndEnd().IsEmpty())
        {
            continue;
        }

        TArray<TSharedPtr<FJsonValue>> MeaningValues;

        for (const FEVVocabularyMeaning& Meaning : Record.Meanings)
        {
            TArray<TSharedPtr<FJsonValue>> DefinitionValues;

            for (const FEVVocabularyDefinition& Definition : Meaning.Definitions)
            {
                const FString DefinitionText = Definition.DefinitionText.TrimStartAndEnd();

                if (DefinitionText.IsEmpty())
                {
                    continue;
                }

                TSharedRef<FJsonObject> DefinitionObject = MakeShared<FJsonObject>();
                DefinitionObject->SetStringField(TEXT("definitionText"), DefinitionText);
                DefinitionObject->SetNumberField(TEXT("definitionDisplayOrder"), Definition.DisplayOrder);
                DefinitionValues.Add(MakeShared<FJsonValueObject>(DefinitionObject));
            }

            if (DefinitionValues.IsEmpty())
            {
                continue;
            }

            FString PartOfSpeech = Meaning.PartOfSpeech.TrimStartAndEnd();
            if (PartOfSpeech.IsEmpty())
            {
                PartOfSpeech = TEXT("unspecified");
            }

            TSharedRef<FJsonObject> MeaningObject = MakeShared<FJsonObject>();
            MeaningObject->SetStringField(TEXT("partOfSpeech"), PartOfSpeech);
            MeaningObject->SetNumberField(TEXT("meaningDisplayOrder"), Meaning.DisplayOrder);
            MeaningObject->SetArrayField(TEXT("definitions"), DefinitionValues);
            MeaningValues.Add(MakeShared<FJsonValueObject>(MeaningObject));
        }

        if (MeaningValues.IsEmpty())
        {
            continue;
        }

        TSharedRef<FJsonObject> RecordObject = MakeShared<FJsonObject>();
        RecordObject->SetStringField(TEXT("word"), Record.Word.TrimStartAndEnd());
        RecordObject->SetStringField(TEXT("normalizedWord"), Record.NormalizedWord.TrimStartAndEnd());
        RecordObject->SetStringField(TEXT("transcription"), ResolveNotificationTranscription(Record));
        RecordObject->SetArrayField(TEXT("meanings"), MeaningValues);
        RecordValues.Add(MakeShared<FJsonValueObject>(RecordObject));
    }

    if (RecordValues.IsEmpty())
    {
        return false;
    }

    TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
    RootObject->SetArrayField(TEXT("records"), RecordValues);

    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutPayload);
    return FJsonSerializer::Serialize(RootObject, Writer) && !OutPayload.IsEmpty();
}

bool LoadNotificationRecords(UEVVocabularyStorageService* StorageService, TArray<FEVVocabularyRecord>& OutRecords)
{
    OutRecords.Reset();

    if (!StorageService)
    {
        return false;
    }

    const int32 EntryCount = StorageService->GetVocabularyEntryCount();
    if (EntryCount <= 0)
    {
        return false;
    }

    const TArray<FVocabularyEntry> Entries = StorageService->GetVocabularyEntries(EntryCount);
    OutRecords.Reserve(Entries.Num());

    for (const FVocabularyEntry& Entry : Entries)
    {
        FEVVocabularyRecord Record;
        if (StorageService->GetVocabularyRecordByWord(
                Entry.NormalizedWord.IsEmpty() ? Entry.Word : Entry.NormalizedWord, Record))
        {
            OutRecords.Add(MoveTemp(Record));
        }
    }

    return !OutRecords.IsEmpty();
}
} // namespace

void UEVNotificationApplicationCoordinator::Initialize(UEVVocabularyStorageService* InStorageService,
                                                       UEVDeviceService* InDeviceService)
{
    VocabularyStorageService = InStorageService;
    DeviceService = InDeviceService;
    if (DeviceService)
    {
        DeviceService->OnPopUpTimerExpired.RemoveAll(this);
        DeviceService->OnPopUpTimerExpired.AddUObject(this, &ThisClass::HandlePopUpTimerExpired);
        DeviceService->OnNotificationPermissionResult().RemoveAll(this);
        DeviceService->OnNotificationPermissionResult().AddUObject(this,
                                                                   &ThisClass::HandleNotificationPermissionResult);
    }
}

void UEVNotificationApplicationCoordinator::Shutdown()
{
    if (DeviceService)
    {
        DeviceService->OnPopUpTimerExpired.RemoveAll(this);
        DeviceService->OnNotificationPermissionResult().RemoveAll(this);
    }
    VocabularyStorageService = nullptr;
    DeviceService = nullptr;
}

bool UEVNotificationApplicationCoordinator::ApplySettings(const FEVPopUpSettingsInfo& PopUpSettings)
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot apply notification settings: DeviceService is invalid."));
        return false;
    }

    CurrentSettings = PopUpSettings;

    const int32 IntervalSeconds = FEVPopUpSettingsInfo::GetIntervalSeconds(PopUpSettings.PopUpIntervals);

    if (IntervalSeconds <= 0)
    {
        return DeviceService->CancelVocabularyNotifications();
    }

#if PLATFORM_ANDROID
    switch (PopUpSettings.NotificationMode)
    {
    case EEVNotificationMode::RandomWord:
    {
        if (!VocabularyStorageService)
        {
            UE_LOG(LogTemp, Error, TEXT("Cannot schedule RandomWord notifications: vocabulary storage is invalid."));
            return false;
        }

        TArray<FEVVocabularyRecord> Records;
        if (!LoadNotificationRecords(VocabularyStorageService, Records))
        {
            UE_LOG(LogTemp, Warning,
                   TEXT("Cannot schedule RandomWord notifications: vocabulary database has no readable records."));
            return false;
        }

        FString SerializedPayload;
        if (!BuildRandomWordNotificationPayload(Records, SerializedPayload))
        {
            UE_LOG(LogTemp, Warning,
                   TEXT("Cannot schedule RandomWord notifications: no word contains a usable definition."));
            return false;
        }

        return DeviceService->ScheduleVocabularyNotifications(IntervalSeconds, SerializedPayload,
                                                              static_cast<int32>(PopUpSettings.NotificationMode));
    }

    case EEVNotificationMode::TestMode:
        return DeviceService->ScheduleVocabularyNotifications(IntervalSeconds, TEXT(""),
                                                              static_cast<int32>(PopUpSettings.NotificationMode));

    default:
        ensureMsgf(false, TEXT("Unsupported notification mode."));
        return false;
    }
#else
    return DeviceService->StartPopUpTimer(IntervalSeconds);
#endif
}

void UEVNotificationApplicationCoordinator::HandlePopUpTimerExpired()
{
    if (CurrentSettings.NotificationMode != EEVNotificationMode::RandomWord)
    {
        return;
    }

    TArray<FEVVocabularyRecord> Records;
    if (!LoadNotificationRecords(VocabularyStorageService, Records))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load vocabulary records for the notification."));
        return;
    }

    FEVRandomWordNotificationSelection Selection;
    if (!SelectRandomWordNotification(Records, Selection))
    {
        UE_LOG(LogTemp, Warning, TEXT("No vocabulary record contains a usable notification definition."));
        return;
    }

    if (!DeviceService->ShowVocabularyNotification(Selection.Word, Selection.NormalizedWord, Selection.Transcription,
                                                   Selection.PartOfSpeech, Selection.MeaningDisplayOrder,
                                                   Selection.DefinitionText, Selection.DefinitionDisplayOrder,
                                                   static_cast<int32>(EEVNotificationMode::RandomWord)))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to show vocabulary notification."));
        return;
    }

    const int32 IntervalSeconds = FEVPopUpSettingsInfo::GetIntervalSeconds(CurrentSettings.PopUpIntervals);

    if (IntervalSeconds > 0)
    {
        DeviceService->StartPopUpTimer(IntervalSeconds);
    }
}

bool UEVNotificationApplicationCoordinator::GetStoredSettings(FEVPopUpSettingsInfo& OutSettings) const
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot read stored notification settings: DeviceService is nullptr."));
        return false;
    }

    bool bEnabled = false;
    int32 IntervalSeconds = 0;
    int32 NotificationModeValue = 0;

    if (!DeviceService->GetStoredVocabularyNotificationSettings(bEnabled, IntervalSeconds, NotificationModeValue))
    {
        return false;
    }

    OutSettings = FEVPopUpSettingsInfo();

    if (bEnabled)
    {
        bool bFoundInterval = false;
        for (const EEVPopUpIntervals Interval : FEVPopUpSettingsInfo::GetAllIntervals())
        {
            if (FEVPopUpSettingsInfo::GetIntervalSeconds(Interval) == IntervalSeconds)
            {
                OutSettings.PopUpIntervals = Interval;
                bFoundInterval = true;
                break;
            }
        }

        if (!bFoundInterval)
        {
            UE_LOG(LogTemp, Warning, TEXT("Stored notification interval %d seconds is unsupported."), IntervalSeconds);
            OutSettings.PopUpIntervals = EEVPopUpIntervals::TurnedOff;
        }
    }

    bool bFoundMode = false;
    for (const EEVNotificationMode Mode : FEVPopUpSettingsInfo::GetAllNotificationModes())
    {
        if (static_cast<int32>(Mode) == NotificationModeValue)
        {
            OutSettings.NotificationMode = Mode;
            bFoundMode = true;
            break;
        }
    }

    if (!bFoundMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("Stored notification mode %d is unsupported."), NotificationModeValue);
    }

    return true;
}

bool UEVNotificationApplicationCoordinator::ConsumePendingNotificationWord(FString& OutWord) const
{
    OutWord.Empty();
    return DeviceService && DeviceService->ConsumePendingNotificationWord(OutWord);
}

bool UEVNotificationApplicationCoordinator::AreNotificationsEnabled() const
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot check notifications: DeviceService is nullptr."));

        return false;
    }

    return DeviceService->AreNotificationsEnabled();
}

bool UEVNotificationApplicationCoordinator::HasRequestedNotificationPermission() const
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot check notification permission history: DeviceService is nullptr."));

        return false;
    }

    return DeviceService->HasRequestedNotificationPermission();
}

bool UEVNotificationApplicationCoordinator::RequestNotificationPermission()
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot request notification permission: DeviceService is nullptr."));

        return false;
    }

    return DeviceService->RequestNotificationPermission();
}

void UEVNotificationApplicationCoordinator::OpenNotificationSettings()
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot open notification settings: DeviceService is nullptr."));

        return;
    }

    DeviceService->OpenNotificationSettings();
}

void UEVNotificationApplicationCoordinator::TestDeviceAlarm()
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot open notification settings: DeviceService is nullptr."));

        return;
    }

    DeviceService->TestAlarm();
}

void UEVNotificationApplicationCoordinator::HandleNotificationPermissionResult(const bool bGranted)
{
    NotificationPermissionResultDelegate.Broadcast(bGranted);
}
