// Fill out your copyright notice in the Description page of Project Settings.

#include "EVGameInstance.h"

#include "EVFileExchangeDefaults.h"

#include "EVVocabularyStorageService.h"
#include "EVWordSearchService.h"
#include "EVConnectivityService.h"
#include "EVDeviceService.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/CoreDelegates.h"

namespace
{
bool TryResolveWebProviderId(const FName ProviderId, const EEVWebProvider DefaultProvider, EEVWebProvider& OutProvider)
{
    if (ProviderId.IsNone())
    {
        OutProvider = DefaultProvider;
        return true;
    }

    if (ProviderId == FName(TEXT("FreeDictionary")))
    {
        OutProvider = EEVWebProvider::FreeDictionary;
        return true;
    }
    if (ProviderId == FName(TEXT("MyMemory")))
    {
        OutProvider = EEVWebProvider::MyMemory;
        return true;
    }
    if (ProviderId == FName(TEXT("Datamuse")))
    {
        OutProvider = EEVWebProvider::Datamuse;
        return true;
    }

    return false;
}

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

void UEVGameInstance::Init()
{
    Super::Init();

    LoadVocabularyLanguagePreferences();
    ApplicationWillEnterBackgroundHandle = FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject(
        this, &ThisClass::HandleApplicationWillEnterBackground);

    VocabularyStorageService = NewObject<UEVVocabularyStorageService>(this);

    WordSearchService = NewObject<UEVWordSearchService>(this);

    ConnectivityService = NewObject<UEVConnectivityService>(this);

    DeviceService = NewObject<UEVDeviceService>(this);

    if (WordSearchService)
    {
        WordSearchService->Initialize();

        WordSearchService->OnEVWordSearchCompleted.AddDynamic(
            this, &ThisClass::HandleEVWordSearchCompletedFromEVGameInstance);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("WordSearchService is null"));
    }

    if (ConnectivityService)
    {
        ConnectivityService->Initialize();
        ConnectivityService->RefreshConnection();

        ConnectivityService->OnConnectionStateChanged.AddDynamic(this, &ThisClass::HandleConnectionStateChanged);

        if (UWorld* World = GetWorld())
        {
            ConnectivityService->StartConnectionPolling(World);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("World is null; connection polling not started"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ConnectivityService is null"));
    }

    if (VocabularyStorageService)
    {
        VocabularyStorageService->InitializeStorage(VocabularyLanguagePreferences.DatabaseContext);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));
    }

    if (DeviceService)
    {
        DeviceService->InitializeDeviceService();

        DeviceService->OnFileSaved().AddUObject(this, &ThisClass::HandleFileSaved);

        DeviceService->OnImportFilePicked().AddUObject(this, &ThisClass::HandleImportFilePicked);

        DeviceService->OnPopUpTimerExpired.AddUObject(this, &ThisClass::HandlePopUpTimerExpired);

        DeviceService->OnNotificationPermissionResult().AddUObject(this,
                                                                   &ThisClass::HandleNotificationPermissionResult);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DeviceService is null"));
    }
}

void UEVGameInstance::Shutdown()
{
    SaveVocabularyLanguagePreferences();
    if (ApplicationWillEnterBackgroundHandle.IsValid())
    {
        FCoreDelegates::ApplicationWillEnterBackgroundDelegate.Remove(ApplicationWillEnterBackgroundHandle);
    }

    if (VocabularyStorageService)
    {
        VocabularyStorageService->ShutdownStorage();
    }

    if (ConnectivityService)
    {
        if (UWorld* World = GetWorld())
        {
            ConnectivityService->StopConnectionPolling(World);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("World is null; connection polling not stopped"));
        }

        ConnectivityService->Shutdown();
    }

    Super::Shutdown();
}

EEVVocabularyStorageServiceResult UEVGameInstance::DoesWordExist(const FString& Word, FText& OutErrorMessage)
{
    OutErrorMessage = FText::GetEmpty();

    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));

        return EEVVocabularyStorageServiceResult::VocabularyStorageInstanceError;
    }

    switch (VocabularyStorageService->DoesWordExist(Word, OutErrorMessage))
    {
    case EEVWordLookupResult::DatabaseError:
        return EEVVocabularyStorageServiceResult::DatabaseError;

    case EEVWordLookupResult::Exists:
        return EEVVocabularyStorageServiceResult::WordExists;

    case EEVWordLookupResult::DoesNotExist:
        return EEVVocabularyStorageServiceResult::WordDoesNotExist;

    default:
        break;
    }

    return EEVVocabularyStorageServiceResult::Empty;
}

bool UEVGameInstance::SaveVocabularyEntry(const FWordSearchResult& WordSearchResult)
{
    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));

        return false;
    }

    if (!WordSearchResult.VocabularyRecord.NormalizedWord.IsEmpty())
    {
        return VocabularyStorageService->SaveVocabularyRecord(WordSearchResult.VocabularyRecord);
    }

    FVocabularyEntry Entry;
    Entry.Word = WordSearchResult.Word;
    Entry.NormalizedWord = WordSearchResult.NormalizedWord;
    Entry.Transcription = WordSearchResult.Transcription;
    Entry.Definition = WordSearchResult.Definition;
    Entry.Usage = WordSearchResult.Usage;
    Entry.TranslationRu = WordSearchResult.TranslationRu;
    Entry.TranslationUa = WordSearchResult.TranslationUa;

    return VocabularyStorageService->SaveVocabularyEntry(Entry);
}

bool UEVGameInstance::UpdateVocabularyEntry(const FVocabularyEntry& Entry, FVocabularyEntry& OutEntry)
{
    OutEntry = FVocabularyEntry();

    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));

        return false;
    }

    if (!VocabularyStorageService->UpdateVocabularyEntry(Entry))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to update vocabulary entry: %s"), *Entry.Word);

        return false;
    }

    return VocabularyStorageService->GetVocabularyEntryByWord(Entry.Word, OutEntry);
}

bool UEVGameInstance::UpdateVocabularyRecord(const FEVVocabularyRecord& Record, FEVVocabularyRecord& OutRecord)
{
    OutRecord = FEVVocabularyRecord{};

    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));
        return false;
    }

    if (!VocabularyStorageService->UpdateVocabularyRecord(Record))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to update vocabulary record: %s"), *Record.Word);
        return false;
    }

    return VocabularyStorageService->GetVocabularyRecordByWord(
        Record.NormalizedWord.IsEmpty() ? Record.Word : Record.NormalizedWord, OutRecord);
}

bool UEVGameInstance::GetVocabularyEntryByWord(const FString& Word, FVocabularyEntry& OutEntry) const
{
    OutEntry = FVocabularyEntry();
    return VocabularyStorageService && VocabularyStorageService->GetVocabularyEntryByWord(Word, OutEntry);
}

bool UEVGameInstance::GetVocabularyRecordByWord(const FString& Word, FEVVocabularyRecord& OutRecord) const
{
    OutRecord = FEVVocabularyRecord{};
    return VocabularyStorageService && VocabularyStorageService->GetVocabularyRecordByWord(Word, OutRecord);
}

bool UEVGameInstance::DeleteVocabularyEntry(const FVocabularyEntry& Entry)
{
    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));

        return false;
    }

    if (!VocabularyStorageService->DeleteVocabularyEntry(Entry))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to delete vocabulary entry: %s"), *Entry.Word);

        return false;
    }

    return true;
}

int32 UEVGameInstance::GetVocabularyEntryCount() const
{
    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("GetVocabularyEntryCount: VocabularyStorageService is null"));

        return 0;
    }

    return VocabularyStorageService->GetVocabularyEntryCount();
}

bool UEVGameInstance::GetVocabularyEntriesPage(TArray<FVocabularyEntry>& OutVocabularyEntries, int32 Limit,
                                               int32 Offset) const
{
    OutVocabularyEntries.Reset();

    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("GetVocabularyEntriesPage: VocabularyStorageService is null"));

        return false;
    }

    if (Limit <= 0 || Offset < 0)
    {
        UE_LOG(LogTemp, Error, TEXT("GetVocabularyEntriesPage received invalid arguments. Limit=%d | Offset=%d"), Limit,
               Offset);

        return false;
    }

    OutVocabularyEntries = VocabularyStorageService->GetVocabularyEntriesPage(Limit, Offset);

    return true;
}

bool UEVGameInstance::GetVocabularyEntries(TArray<FVocabularyEntry>& OutVocabularyEntries, int32 EntryNumber)
{
    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("VocabularyStorageService is null"));

        return false;
    }

    OutVocabularyEntries = VocabularyStorageService->GetVocabularyEntries(EntryNumber);

    return true;
}

int32 UEVGameInstance::GetVocabularyEntryCountByPrefix(const FString& SearchPrefix) const
{
    if (!VocabularyStorageService)
    {
        return 0;
    }

    return VocabularyStorageService->GetVocabularyEntryCountByPrefix(SearchPrefix);
}

bool UEVGameInstance::GetVocabularyEntriesPageByPrefix(TArray<FVocabularyEntry>& OutVocabularyEntries,
                                                       const FString& SearchPrefix, int32 Limit, int32 Offset) const
{
    OutVocabularyEntries.Reset();

    if (!VocabularyStorageService)
    {
        return false;
    }

    OutVocabularyEntries = VocabularyStorageService->GetVocabularyEntriesPageByPrefix(SearchPrefix, Limit, Offset);

    return true;
}

bool UEVGameInstance::GetRandomlySelectedWord(FString& OutWord)
{
    OutWord.Reset();

    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot retrieve random word: vocabulary storage service is invalid."));

        return false;
    }

    return VocabularyStorageService->GetRandomlySelectedWord(OutWord);
}

bool UEVGameInstance::HandlePopUpIntervalSelected(const FEVPopUpSettingsInfo& PopUpSettings)
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot apply notification settings: DeviceService is invalid."));
        return false;
    }

    CurrentPopUpSettings = PopUpSettings;

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

FWordSearchResult UEVGameInstance::SearchWordFake(const FString& Word)
{
    if (!WordSearchService)
    {
        FWordSearchResult Result;
        Result.bSuccess = false;
        Result.ErrorMessage = TEXT("WordSearchService/Fake search is null");

        return Result;
    }

    FWordSearchResult Result = WordSearchService->SearchWordFake(Word);

    auto IsSelectedTarget = [this](const FString& TargetLanguage)
    {
        EEVVocabularyTranslationLanguage Parsed = EEVVocabularyTranslationLanguage::None;
        return EVVocabularyLanguage::TryParseTranslationLanguage(TargetLanguage, Parsed) &&
               VocabularyLanguagePreferences.SelectedTranslations.Contains(Parsed);
    };

    Result.VocabularyRecord.GeneralTranslations.RemoveAll(
        [&IsSelectedTarget](const FEVVocabularyTranslation& Translation)
        { return !IsSelectedTarget(Translation.TargetLanguage); });
    for (FEVVocabularyMeaning& Meaning : Result.VocabularyRecord.Meanings)
    {
        Meaning.Translations.RemoveAll([&IsSelectedTarget](const FEVVocabularyTranslation& Translation)
                                       { return !IsSelectedTarget(Translation.TargetLanguage); });
    }

    if (!VocabularyLanguagePreferences.SelectedTranslations.Contains(EEVVocabularyTranslationLanguage::Russian))
    {
        Result.TranslationRu.Reset();
    }
    if (!VocabularyLanguagePreferences.SelectedTranslations.Contains(EEVVocabularyTranslationLanguage::Ukrainian))
    {
        Result.TranslationUa.Reset();
    }

    return Result;
}

void UEVGameInstance::SearchWordOnline(const FString& Word, EEVWebProvider DefinitionUsageProvider,
                                       EEVWebProvider TranslationProvider)
{
    if (!WordSearchService)
    {
        UE_LOG(LogTemp, Error, TEXT("WordSearchService/Real Online Search is null"));

        return;
    }

    WordSearchService->SearchWordOnline(Word, DefinitionUsageProvider, TranslationProvider,
                                        VocabularyLanguagePreferences.DatabaseContext,
                                        VocabularyLanguagePreferences.SelectedTranslations);
}

void UEVGameInstance::RequestVocabularySearch(const FEVVocabularySearchRequest& Request)
{
    FEVVocabularySearchOutcome Outcome;
    Outcome.RequestId = Request.RequestId;

    if (!Request.RequestId.IsValid() || Request.Word.TrimStartAndEnd().IsEmpty())
    {
        Outcome.Result = EEVApplicationOperationResult::Failed;
        Outcome.Message = FText::FromString(TEXT("Vocabulary search request is invalid."));
        VocabularySearchOutcomeReadyDelegate.Broadcast(Outcome);
        return;
    }

    if (PendingVocabularySearchRequestId.IsValid())
    {
        Outcome.Result = EEVApplicationOperationResult::Busy;
        Outcome.Message = FText::FromString(TEXT("A vocabulary search is already in progress."));
        VocabularySearchOutcomeReadyDelegate.Broadcast(Outcome);
        return;
    }

    if (!WordSearchService)
    {
        Outcome.Result = EEVApplicationOperationResult::Unavailable;
        Outcome.Message = FText::FromString(TEXT("Vocabulary search is unavailable."));
        VocabularySearchOutcomeReadyDelegate.Broadcast(Outcome);
        return;
    }

    EEVWebProvider DefinitionProvider = EEVWebProvider::FreeDictionary;
    EEVWebProvider TranslationProvider = EEVWebProvider::MyMemory;
    if (!TryResolveWebProviderId(Request.DefinitionProviderId, EEVWebProvider::FreeDictionary, DefinitionProvider) ||
        !TryResolveWebProviderId(Request.TranslationProviderId, EEVWebProvider::MyMemory, TranslationProvider))
    {
        Outcome.Result = EEVApplicationOperationResult::Unavailable;
        Outcome.Message = FText::FromString(TEXT("The requested vocabulary provider is unavailable."));
        VocabularySearchOutcomeReadyDelegate.Broadcast(Outcome);
        return;
    }

    PendingVocabularySearchRequestId = Request.RequestId;
    SearchWordOnline(Request.Word.TrimStartAndEnd(), DefinitionProvider, TranslationProvider);
}

void UEVGameInstance::RequestVocabularyRecord(const FEVVocabularyRecordRequest& Request)
{
    FEVVocabularyRecordOutcome Outcome;
    Outcome.RequestId = Request.RequestId;

    if (!Request.RequestId.IsValid() || Request.Word.TrimStartAndEnd().IsEmpty())
    {
        Outcome.Result = EEVApplicationOperationResult::Failed;
        Outcome.Message = FText::FromString(TEXT("Vocabulary record request is invalid."));
        VocabularyRecordOutcomeReadyDelegate.Broadcast(Outcome);
        return;
    }

    FText ErrorMessage;
    switch (DoesWordExist(Request.Word, ErrorMessage))
    {
    case EEVVocabularyStorageServiceResult::WordDoesNotExist:
        Outcome.Result = EEVApplicationOperationResult::Succeeded;
        Outcome.bExists = false;
        break;

    case EEVVocabularyStorageServiceResult::WordExists:
        Outcome.bExists = GetVocabularyRecordByWord(Request.Word, Outcome.Record);
        Outcome.Result =
            Outcome.bExists ? EEVApplicationOperationResult::Succeeded : EEVApplicationOperationResult::Failed;
        if (!Outcome.bExists)
        {
            Outcome.Message = FText::FromString(TEXT("The vocabulary record could not be loaded."));
        }
        break;

    case EEVVocabularyStorageServiceResult::VocabularyStorageInstanceError:
        Outcome.Result = EEVApplicationOperationResult::Unavailable;
        Outcome.Message = FText::FromString(TEXT("Vocabulary storage is unavailable."));
        break;

    default:
        Outcome.Result = EEVApplicationOperationResult::Failed;
        Outcome.Message =
            ErrorMessage.IsEmpty() ? FText::FromString(TEXT("The vocabulary lookup failed.")) : ErrorMessage;
        break;
    }

    VocabularyRecordOutcomeReadyDelegate.Broadcast(Outcome);
}

void UEVGameInstance::RequestVocabularyQuery(const FEVVocabularyQueryRequest& Request)
{
    FEVVocabularyQueryOutcome Outcome;
    Outcome.RequestId = Request.RequestId;

    if (!Request.RequestId.IsValid() || Request.Limit <= 0 || Request.Offset < 0)
    {
        Outcome.Result = EEVApplicationOperationResult::Failed;
        Outcome.Message = FText::FromString(TEXT("Vocabulary query request is invalid."));
        VocabularyQueryOutcomeReadyDelegate.Broadcast(Outcome);
        return;
    }

    if (!VocabularyStorageService)
    {
        Outcome.Result = EEVApplicationOperationResult::Unavailable;
        Outcome.Message = FText::FromString(TEXT("Vocabulary storage is unavailable."));
        VocabularyQueryOutcomeReadyDelegate.Broadcast(Outcome);
        return;
    }

    FEVVocabularyQueryCriteria Criteria = Request.Criteria;
    Criteria.Normalize();
    Outcome.TotalEntries = GetVocabularyEntryCountByCriteria(Request.SearchPrefix, Criteria);

    TArray<FVocabularyEntry> Entries;
    if (!GetVocabularyEntriesPageByCriteria(Entries, Request.SearchPrefix, Criteria, Request.Limit, Request.Offset))
    {
        Outcome.Result = EEVApplicationOperationResult::Failed;
        Outcome.Message = FText::FromString(TEXT("The vocabulary query failed."));
        VocabularyQueryOutcomeReadyDelegate.Broadcast(Outcome);
        return;
    }

    Outcome.Records.Reserve(Entries.Num());
    for (const FVocabularyEntry& Entry : Entries)
    {
        FEVVocabularyRecord Record;
        const FString RecordKey = Entry.NormalizedWord.IsEmpty() ? Entry.Word : Entry.NormalizedWord;
        if (!GetVocabularyRecordByWord(RecordKey, Record))
        {
            Outcome.Records.Reset();
            Outcome.Result = EEVApplicationOperationResult::Failed;
            Outcome.Message = FText::FromString(TEXT("A vocabulary record in the query could not be loaded."));
            VocabularyQueryOutcomeReadyDelegate.Broadcast(Outcome);
            return;
        }
        Outcome.Records.Add(MoveTemp(Record));
    }

    Outcome.Result = EEVApplicationOperationResult::Succeeded;
    VocabularyQueryOutcomeReadyDelegate.Broadcast(Outcome);
}

void UEVGameInstance::RequestVocabularyMutation(const FEVVocabularyMutationRequest& Request)
{
    FEVVocabularyMutationOutcome Outcome;
    Outcome.RequestId = Request.RequestId;
    Outcome.MutationType = Request.MutationType;
    Outcome.Record = Request.Record;

    const FString RecordKey =
        Request.Record.NormalizedWord.IsEmpty() ? Request.Record.Word : Request.Record.NormalizedWord;
    if (!Request.RequestId.IsValid() || RecordKey.TrimStartAndEnd().IsEmpty())
    {
        Outcome.Result = EEVApplicationOperationResult::Failed;
        Outcome.Message = FText::FromString(TEXT("Vocabulary mutation request is invalid."));
        VocabularyMutationOutcomeReadyDelegate.Broadcast(Outcome);
        return;
    }

    if (!VocabularyStorageService)
    {
        Outcome.Result = EEVApplicationOperationResult::Unavailable;
        Outcome.Message = FText::FromString(TEXT("Vocabulary storage is unavailable."));
        VocabularyMutationOutcomeReadyDelegate.Broadcast(Outcome);
        return;
    }

    bool bSucceeded = false;
    EEVVocabularyChangeType ChangeType = EEVVocabularyChangeType::BulkChanged;
    switch (Request.MutationType)
    {
    case EEVVocabularyMutationType::Save:
        bSucceeded = VocabularyStorageService->SaveVocabularyRecord(Request.Record);
        ChangeType = EEVVocabularyChangeType::Added;
        break;

    case EEVVocabularyMutationType::Update:
        bSucceeded = VocabularyStorageService->UpdateVocabularyRecord(Request.Record);
        ChangeType = EEVVocabularyChangeType::Updated;
        break;

    case EEVVocabularyMutationType::Delete:
        bSucceeded = VocabularyStorageService->DeleteVocabularyRecord(RecordKey);
        ChangeType = EEVVocabularyChangeType::Removed;
        break;
    }

    Outcome.Result = bSucceeded ? EEVApplicationOperationResult::Succeeded : EEVApplicationOperationResult::Failed;
    if (!bSucceeded)
    {
        Outcome.Message = FText::FromString(TEXT("The vocabulary mutation failed."));
        VocabularyMutationOutcomeReadyDelegate.Broadcast(Outcome);
        return;
    }

    if (Request.MutationType != EEVVocabularyMutationType::Delete)
    {
        FEVVocabularyRecord StoredRecord;
        if (GetVocabularyRecordByWord(RecordKey, StoredRecord))
        {
            Outcome.Record = MoveTemp(StoredRecord);
        }
    }

    VocabularyMutationOutcomeReadyDelegate.Broadcast(Outcome);

    FEVVocabularyChangeInfo ChangeInfo;
    ChangeInfo.ChangeId = FGuid::NewGuid();
    ChangeInfo.CorrelationId = Request.RequestId;
    ChangeInfo.OriginId = FName(TEXT("LocalApplication"));
    ChangeInfo.ChangeType = ChangeType;
    ChangeInfo.AffectedNormalizedWords.Add(Outcome.Record.NormalizedWord.IsEmpty() ? RecordKey
                                                                                   : Outcome.Record.NormalizedWord);
    VocabularyChangedDelegate.Broadcast(ChangeInfo);
}

void UEVGameInstance::RequestVocabularyPreferencesChange(const FEVVocabularyPreferencesChangeRequest& Request)
{
    FEVVocabularyPreferencesState State;
    State.RequestId = Request.RequestId;

    if (!Request.RequestId.IsValid())
    {
        State.Result = EEVApplicationOperationResult::Failed;
        State.Preferences = VocabularyLanguagePreferences;
        State.Message = FText::FromString(TEXT("Vocabulary preferences request is invalid."));
        VocabularyPreferencesStateReadyDelegate.Broadcast(State);
        return;
    }

    const bool bApplied = SetVocabularyLanguagePreferences(Request.Preferences);
    State.Result = bApplied ? EEVApplicationOperationResult::Succeeded : EEVApplicationOperationResult::Failed;
    State.Preferences = VocabularyLanguagePreferences;
    if (!bApplied)
    {
        State.Message = FText::FromString(TEXT("Vocabulary preferences could not be applied."));
    }
    VocabularyPreferencesStateReadyDelegate.Broadcast(State);
}

FEVRequestedActionInfo UEVGameInstance::HandleFileOperationRequested(const FEVFileOperationInfo& FileOperationInfo)
{
    switch (FileOperationInfo.OperationType)
    {
    case EEVFileOperationType::DownloadTemplate:
        return HandleDownloadTemplateRequested(FileOperationInfo);

    case EEVFileOperationType::ExportDB:
        return HandleExportDBRequested(FileOperationInfo);

    case EEVFileOperationType::ImportDBOverwrite:
        return HandleImportDBOverwriteRequested(FileOperationInfo);

    case EEVFileOperationType::ImportDBAppend:
        return HandleImportDBAppendRequested(FileOperationInfo);

    default:
    {
        FEVRequestedActionInfo ActionInfo;
        ActionInfo.Source = EEVRequestedActionSource::ImportExport;
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("Unsupported file operation."));
        ActionInfo.GenerateColor();

        return ActionInfo;
    }
    }
}

const FEVVocabularyLanguagePreferences& UEVGameInstance::GetVocabularyLanguagePreferences() const
{
    return VocabularyLanguagePreferences;
}

bool UEVGameInstance::SetVocabularyLanguagePreferences(const FEVVocabularyLanguagePreferences& Preferences)
{
    FEVVocabularyLanguagePreferences Normalized = Preferences;
    Normalized.Normalize();

    const FEVVocabularyLanguagePreferences Previous = VocabularyLanguagePreferences;
    const bool bContextChanged = Normalized.DatabaseContext != Previous.DatabaseContext;

    if (bContextChanged && VocabularyStorageService)
    {
        VocabularyStorageService->ShutdownStorage();
        if (!VocabularyStorageService->InitializeStorage(Normalized.DatabaseContext))
        {
            UE_LOG(LogTemp, Error,
                   TEXT("Failed to initialize storage for vocabulary language context. Restoring previous context."));
            VocabularyStorageService->InitializeStorage(Previous.DatabaseContext);
            VocabularyLanguagePreferences = Previous;
            SaveVocabularyLanguagePreferences();
            return false;
        }
    }

    VocabularyLanguagePreferences = MoveTemp(Normalized);
    SaveVocabularyLanguagePreferences();
    return true;
}

void UEVGameInstance::LoadVocabularyLanguagePreferences()
{
    static const TCHAR* Section = TEXT("EnhanceVocabulary.LanguagePreferences");

    FEVVocabularyLanguagePreferences Loaded;

    FString ContextValue;
    if (GConfig && GConfig->GetString(Section, TEXT("DatabaseContext"), ContextValue, GGameUserSettingsIni))
    {
        EEVVocabularyDBContext ParsedContext = EEVVocabularyDBContext::None;
        if (EVVocabularyLanguage::TryParseDatabaseContext(ContextValue, ParsedContext))
        {
            Loaded.DatabaseContext = ParsedContext;
        }
    }

    FString TranslationValues;
    const bool bHasPersistedTranslations =
        GConfig && GConfig->GetString(Section, TEXT("TranslationLanguages"), TranslationValues, GGameUserSettingsIni);

    if (bHasPersistedTranslations)
    {
        Loaded.SelectedTranslations.Reset();
        TArray<FString> Tokens;
        TranslationValues.ParseIntoArray(Tokens, TEXT(","), true);
        for (const FString& Token : Tokens)
        {
            EEVVocabularyTranslationLanguage Language = EEVVocabularyTranslationLanguage::None;
            if (EVVocabularyLanguage::TryParseTranslationLanguage(Token, Language))
            {
                Loaded.SelectedTranslations.Add(Language);
            }
        }
    }

    Loaded.Normalize();
    VocabularyLanguagePreferences = MoveTemp(Loaded);
}

void UEVGameInstance::SaveVocabularyLanguagePreferences() const
{
    if (!GConfig)
    {
        return;
    }

    static const TCHAR* Section = TEXT("EnhanceVocabulary.LanguagePreferences");

    GConfig->SetString(Section, TEXT("DatabaseContext"),
                       *EVVocabularyLanguage::GetDatabaseContextId(VocabularyLanguagePreferences.DatabaseContext),
                       GGameUserSettingsIni);

    TArray<FString> TranslationIds;
    for (const EEVVocabularyTranslationLanguage Language : VocabularyLanguagePreferences.SelectedTranslations)
    {
        const FString Code = EVVocabularyLanguage::GetTranslationStorageCode(Language);
        if (!Code.IsEmpty())
        {
            TranslationIds.Add(Code);
        }
    }
    GConfig->SetString(Section, TEXT("TranslationLanguages"), *FString::Join(TranslationIds, TEXT(",")),
                       GGameUserSettingsIni);
    GConfig->Flush(false, GGameUserSettingsIni);
}

void UEVGameInstance::HandleApplicationWillEnterBackground()
{
    SaveVocabularyLanguagePreferences();
}

EEVConnectionState UEVGameInstance::GetConnectionState() const
{
    if (ConnectivityService)
    {
        return ConnectivityService->GetConnectionState();
    }

    UE_LOG(LogTemp, Error, TEXT("ConnectivityService is null in UEVGameInstance::GetConnectionState()"));

    return EEVConnectionState::Offline;
}

void UEVGameInstance::HandleConnectionStateChanged(EEVConnectionState NewState)
{
    if (EVConnectionState == NewState)
    {
        return;
    }

    EVConnectionState = NewState;

    OnConnectionStateChanged.Broadcast(NewState);
}

void UEVGameInstance::HandleEVWordSearchCompletedFromEVGameInstance(
    const FWordSearchResult& SearchWordResultPassedByGameInstance)
{
    const FGuid CompletedRequestId = PendingVocabularySearchRequestId;
    PendingVocabularySearchRequestId = FGuid();

    OnEVWordSearchCompletedFromEVGameInstance.Broadcast(SearchWordResultPassedByGameInstance);

    if (CompletedRequestId.IsValid())
    {
        FEVVocabularySearchOutcome Outcome;
        Outcome.RequestId = CompletedRequestId;
        Outcome.Result = SearchWordResultPassedByGameInstance.bSuccess ? EEVApplicationOperationResult::Succeeded
                                                                       : EEVApplicationOperationResult::Failed;
        Outcome.Record = SearchWordResultPassedByGameInstance.VocabularyRecord;
        Outcome.Message = FText::FromString(SearchWordResultPassedByGameInstance.ErrorMessage);
        VocabularySearchOutcomeReadyDelegate.Broadcast(Outcome);
    }
}

void UEVGameInstance::HandlePopUpTimerExpired()
{
    if (CurrentPopUpSettings.NotificationMode != EEVNotificationMode::RandomWord)
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

    const int32 IntervalSeconds = FEVPopUpSettingsInfo::GetIntervalSeconds(CurrentPopUpSettings.PopUpIntervals);

    if (IntervalSeconds > 0)
    {
        DeviceService->StartPopUpTimer(IntervalSeconds);
    }
}

void UEVGameInstance::HandleFileSaved(const FEVFileExchangeResultInfo& ResultInfo)
{
    UE_LOG(LogTemp, Log, TEXT("File saved result: %d | File: %s | Bytes: %lld | Message: %s | Debug: %s"),
           static_cast<int32>(ResultInfo.Result), *ResultInfo.FileName, ResultInfo.ByteSize, *ResultInfo.UserMessage,
           *ResultInfo.DebugMessage);

    /*
     * A successfully saved validation report does not mean that
     * the database import succeeded.
     *
     * The import remains failed because its source data did not
     * pass validation.
     */
    if (PendingFileSavePurpose == EEVPendingFileSavePurpose::ImportValidationReport)
    {
        FEVRequestedActionInfo ActionInfo;
        ActionInfo.Source = EEVRequestedActionSource::ImportExport;

        switch (PendingImportFileOperationInfo.OperationType)
        {
        case EEVFileOperationType::ImportDBAppend:
            ActionInfo.Type = EEVRequestedActionType::ImportDBAppend;
            break;

        case EEVFileOperationType::ImportDBOverwrite:
        default:
            ActionInfo.Type = EEVRequestedActionType::Unknown;
            break;
        }

        ActionInfo.Status = EEVRequestedActionStatus::Failed;

        if (ResultInfo.IsSuccess())
        {
            ActionInfo.Message = FText::FromString(PendingImportValidationResult.UserMessage +
                                                   TEXT(" The validation report was saved successfully."));
        }
        else if (ResultInfo.Result == EEVFileExchangeResult::CancelledByUser)
        {
            ActionInfo.Message = FText::FromString(PendingImportValidationResult.UserMessage +
                                                   TEXT(" Saving the validation report was cancelled."));
        }
        else
        {
            ActionInfo.Message = FText::FromString(PendingImportValidationResult.UserMessage +
                                                   TEXT(" The validation report could not be saved."));
        }

        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Warning,
               TEXT("Import validation failed. Report save result: %d | Validation debug: %s | Save debug: %s"),
               static_cast<int32>(ResultInfo.Result), *PendingImportValidationResult.DebugMessage,
               *ResultInfo.DebugMessage);

        PendingFileSavePurpose = EEVPendingFileSavePurpose::None;

        PendingImportValidationResult = FEVFileExchangeResultInfo();

        PendingImportFileOperationInfo = FEVFileOperationInfo();

        FileOperationCompletedDelegate.Broadcast(ActionInfo);

        return;
    }

    /*
     * Normal Template / Export completion.
     */
    FEVFileExchangeResultInfo AdjustedResult = ResultInfo;

    if (ResultInfo.IsSuccess())
    {
        switch (PendingFileSavePurpose)
        {
        case EEVPendingFileSavePurpose::DownloadTemplate:
            AdjustedResult.UserMessage = TEXT("The database template was saved successfully.");
            break;

        case EEVPendingFileSavePurpose::ExportDatabase:
            AdjustedResult.UserMessage = TEXT("The database export was saved successfully.");
            break;

        default:
            break;
        }
    }

    PendingFileSavePurpose = EEVPendingFileSavePurpose::None;

    const FEVRequestedActionInfo ActionInfo = ConvertFileExchangeResultToRequestedAction(AdjustedResult);

    FileOperationCompletedDelegate.Broadcast(ActionInfo);
}

void UEVGameInstance::HandleImportFilePicked(const FEVFileExchangeResultInfo& ResultInfo, const TArray<uint8>& Bytes)
{
    UE_LOG(LogTemp, Log,
           TEXT("Import file picked result: %d | File: %s | Bytes: %lld | ReceivedBytes: %d | Message: %s | Debug: %s"),
           static_cast<int32>(ResultInfo.Result), *ResultInfo.FileName, ResultInfo.ByteSize, Bytes.Num(),
           *ResultInfo.UserMessage, *ResultInfo.DebugMessage);

    if (!ResultInfo.IsSuccess())
    {
        CompleteImportFileOperation(ResultInfo);
        return;
    }

    if (!VocabularyStorageService)
    {
        FEVFileExchangeResultInfo ErrorResult;
        ErrorResult.Result = EEVFileExchangeResult::StorageValidationFailed;
        ErrorResult.UserMessage = TEXT("Vocabulary storage service is not available.");
        ErrorResult.DebugMessage = TEXT("HandleImportFilePicked: VocabularyStorageService is null.");

        PopulateImportResultFileInfo(ErrorResult, ResultInfo, Bytes.Num());

        CompleteImportFileOperation(ErrorResult);
        return;
    }

    TArray<uint8> ValidationReportBytes;
    TArray<FEVVocabularyRecord> ValidatedRecords;

    FEVFileExchangeResultInfo ValidationResult = VocabularyStorageService->ValidateImportFile(
        PendingImportFileOperationInfo.FileExtensionType, PendingImportFileOperationInfo.OperationType, Bytes,
        ValidationReportBytes, ValidatedRecords);

    PopulateImportResultFileInfo(ValidationResult, ResultInfo, Bytes.Num());

    UE_LOG(LogTemp, Log, TEXT("Import validation result: %d | ReportBytes: %d | Message: %s | Debug: %s"),
           static_cast<int32>(ValidationResult.Result), ValidationReportBytes.Num(), *ValidationResult.UserMessage,
           *ValidationResult.DebugMessage);

    if (TrySaveImportValidationReport(ValidationResult, ValidationReportBytes))
    {
        return;
    }

    if (!ValidationResult.IsSuccess())
    {
        CompleteImportFileOperation(ValidationResult);
        return;
    }

    FEVFileExchangeResultInfo DatabaseOperationResult = ExecuteImportDatabaseOperation(ValidatedRecords);

    PopulateImportResultFileInfo(DatabaseOperationResult, ResultInfo, Bytes.Num());

    UE_LOG(LogTemp, Log, TEXT("Import database operation result: %d | ReportBytes: %d | Message: %s | Debug: %s"),
           static_cast<int32>(DatabaseOperationResult.Result), ValidationReportBytes.Num(),
           *DatabaseOperationResult.UserMessage, *DatabaseOperationResult.DebugMessage);

    if (TrySaveImportValidationReport(DatabaseOperationResult, ValidationReportBytes))
    {
        return;
    }

    CompleteImportFileOperation(DatabaseOperationResult);
}

void UEVGameInstance::CompleteImportFileOperation(const FEVFileExchangeResultInfo& ResultInfo)
{
    PendingImportFileOperationInfo = FEVFileOperationInfo();

    ImportFilePickCompletedDelegate.Broadcast(ResultInfo);
}

bool UEVGameInstance::TrySaveImportValidationReport(FEVFileExchangeResultInfo ValidationResult,
                                                    const TArray<uint8>& ValidationReportBytes)
{
    if (ValidationReportBytes.IsEmpty())
    {
        return false;
    }

    if (!DeviceService)
    {
        ValidationResult.Result = EEVFileExchangeResult::UnsupportedPlatform;

        ValidationResult.UserMessage = TEXT("The validation report could not be saved.");

        ValidationResult.DebugMessage = TEXT("TrySaveImportValidationReport: DeviceService is null.");

        CompleteImportFileOperation(ValidationResult);

        return true;
    }

    PendingImportValidationResult = ValidationResult;

    PendingFileSavePurpose = EEVPendingFileSavePurpose::ImportValidationReport;

    UE_LOG(LogTemp, Warning, TEXT("Validation report save requested. Bytes: %d | Validation: %s"),
           ValidationReportBytes.Num(), *ValidationResult.DebugMessage);

    DeviceService->SaveBytesToUserSelectedLocation(
        PendingImportFileOperationInfo.FileExtensionType,
        FEVFileExchangeDefaults::GetValidationReportFileName(PendingImportFileOperationInfo.FileExtensionType),
        ValidationReportBytes);

    return true;
}

FEVFileExchangeResultInfo
UEVGameInstance::ExecuteImportDatabaseOperation(const TArray<FEVVocabularyRecord>& ValidatedRecords)
{
    switch (PendingImportFileOperationInfo.OperationType)
    {
    case EEVFileOperationType::ImportDBOverwrite:
        return VocabularyStorageService->OverwriteDatabase(ValidatedRecords);

    case EEVFileOperationType::ImportDBAppend:
        return VocabularyStorageService->AppendDatabase(ValidatedRecords);

    default:
    {
        FEVFileExchangeResultInfo ResultInfo;
        ResultInfo.Result = EEVFileExchangeResult::StorageValidationFailed;
        ResultInfo.UserMessage = TEXT("The requested import operation is not supported.");
        ResultInfo.DebugMessage = TEXT("ExecuteImportDatabaseOperation received an unknown import operation.");

        return ResultInfo;
    }
    }
}

void UEVGameInstance::PopulateImportResultFileInfo(FEVFileExchangeResultInfo& ResultInfo,
                                                   const FEVFileExchangeResultInfo& PickResult, int32 ByteCount) const
{
    ResultInfo.FileName = PickResult.FileName;

    ResultInfo.ByteSize = ByteCount;
}

FEVRequestedActionInfo UEVGameInstance::HandleDownloadTemplateRequested(const FEVFileOperationInfo& FileOperationInfo)
{
    FEVRequestedActionInfo ActionInfo;
    ActionInfo.Source = EEVRequestedActionSource::ImportExport;
    ActionInfo.Type = EEVRequestedActionType::DownloadDBTemplate;

    if (!VocabularyStorageService)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("Vocabulary storage service is not available."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("HandleDownloadTemplateRequested: VocabularyStorageService is null."));

        return ActionInfo;
    }

    if (!DeviceService)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("Device service is not available."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("HandleDownloadTemplateRequested: DeviceService is null."));

        return ActionInfo;
    }

    TArray<uint8> TemplateBytes;

    const FEVFileExchangeResultInfo TemplateGenerationResult =
        VocabularyStorageService->GenerateDatabaseExportTemplate(FileOperationInfo.FileExtensionType, TemplateBytes);

    if (!TemplateGenerationResult.IsSuccess())
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TemplateGenerationResult.UserMessage);
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("Failed to generate database template: %s"),
               *TemplateGenerationResult.DebugMessage);

        return ActionInfo;
    }

    if (TemplateBytes.IsEmpty())
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("The generated database template is empty."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("GenerateDatabaseExportTemplate succeeded but returned an empty buffer."));

        return ActionInfo;
    }

    PendingFileSavePurpose = EEVPendingFileSavePurpose::DownloadTemplate;

    DeviceService->SaveBytesToUserSelectedLocation(
        FileOperationInfo.FileExtensionType,
        FEVFileExchangeDefaults::GetTemplateFileName(FileOperationInfo.FileExtensionType), TemplateBytes);

    ActionInfo.Status = EEVRequestedActionStatus::InProgress;
    ActionInfo.Message = FText::FromString(TEXT("Preparing template download..."));
    ActionInfo.GenerateColor();

    return ActionInfo;
}

FEVRequestedActionInfo UEVGameInstance::HandleExportDBRequested(const FEVFileOperationInfo& FileOperationInfo)
{
    FEVRequestedActionInfo ActionInfo;
    ActionInfo.Source = EEVRequestedActionSource::ImportExport;
    ActionInfo.Type = EEVRequestedActionType::ExportDB;

    if (!VocabularyStorageService)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("Vocabulary storage service is not available."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("HandleExportDBRequested: VocabularyStorageService is null."));

        return ActionInfo;
    }

    if (!DeviceService)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("Device service is not available."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("HandleExportDBRequested: DeviceService is null."));

        return ActionInfo;
    }

    TArray<uint8> ExportBytes;

    const FEVFileExchangeResultInfo ExportGenerationResult =
        VocabularyStorageService->GenerateDatabaseExport(FileOperationInfo.FileExtensionType, ExportBytes);

    if (!ExportGenerationResult.IsSuccess())
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(ExportGenerationResult.UserMessage);
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("Failed to generate database export: %s"), *ExportGenerationResult.DebugMessage);

        return ActionInfo;
    }

    if (ExportBytes.IsEmpty())
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("The generated database export is empty."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("GenerateDatabaseExport succeeded but returned an empty buffer."));

        return ActionInfo;
    }

    PendingFileSavePurpose = EEVPendingFileSavePurpose::ExportDatabase;

    DeviceService->SaveBytesToUserSelectedLocation(
        FileOperationInfo.FileExtensionType,
        FEVFileExchangeDefaults::GetDatabaseExportFileName(FileOperationInfo.FileExtensionType), ExportBytes);

    ActionInfo.Status = EEVRequestedActionStatus::InProgress;
    ActionInfo.Message = FText::FromString(TEXT("Preparing database export..."));
    ActionInfo.GenerateColor();

    return ActionInfo;
}

FEVRequestedActionInfo UEVGameInstance::HandleImportDBOverwriteRequested(const FEVFileOperationInfo& FileOperationInfo)
{
    FEVRequestedActionInfo ActionInfo;
    ActionInfo.Source = EEVRequestedActionSource::ImportExport;
    ActionInfo.Type = EEVRequestedActionType::ImportDBOverwrite;

    if (!DeviceService)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("Device service is not available."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("HandleImportDBOverwriteRequested: DeviceService is null."));

        return ActionInfo;
    }

    if (FileOperationInfo.FileExtensionType == EEVFileExtensionType::Unknown)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("No import file type was selected."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("HandleImportDBOverwriteRequested received Unknown extension."));

        return ActionInfo;
    }

    PendingImportFileOperationInfo = FileOperationInfo;

    DeviceService->PickImportFile(FileOperationInfo.FileExtensionType);

    ActionInfo.Status = EEVRequestedActionStatus::InProgress;
    ActionInfo.Message = FText::FromString(TEXT("Select a database file to import."));
    ActionInfo.GenerateColor();

    return ActionInfo;
}

FEVRequestedActionInfo UEVGameInstance::HandleImportDBAppendRequested(const FEVFileOperationInfo& FileOperationInfo)
{
    FEVRequestedActionInfo ActionInfo;
    ActionInfo.Source = EEVRequestedActionSource::ImportExport;
    ActionInfo.Type = EEVRequestedActionType::ImportDBAppend;

    if (!DeviceService)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("Device service is not available."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("HandleImportDBAppendRequested: DeviceService is null."));

        return ActionInfo;
    }

    if (FileOperationInfo.FileExtensionType == EEVFileExtensionType::Unknown)
    {
        ActionInfo.Status = EEVRequestedActionStatus::Failed;
        ActionInfo.Message = FText::FromString(TEXT("No import file type was selected."));
        ActionInfo.GenerateColor();

        UE_LOG(LogTemp, Error, TEXT("HandleImportDBAppendRequested received Unknown extension."));

        return ActionInfo;
    }

    PendingImportFileOperationInfo = FileOperationInfo;

    DeviceService->PickImportFile(FileOperationInfo.FileExtensionType);

    ActionInfo.Status = EEVRequestedActionStatus::InProgress;
    ActionInfo.Message = FText::FromString(TEXT("Select a database file to append."));
    ActionInfo.GenerateColor();

    return ActionInfo;
}

FEVFileOperationCompletedFromGameInstance& UEVGameInstance::OnFileOperationCompleted()
{
    return FileOperationCompletedDelegate;
}

FEVRequestedActionInfo
UEVGameInstance::ConvertFileExchangeResultToRequestedAction(const FEVFileExchangeResultInfo& ResultInfo) const
{
    FEVRequestedActionInfo ActionInfo;
    ActionInfo.Source = EEVRequestedActionSource::ImportExport;

    ActionInfo.Status = ResultInfo.IsSuccess() ? EEVRequestedActionStatus::Completed : EEVRequestedActionStatus::Failed;

    ActionInfo.Message = FText::FromString(ResultInfo.UserMessage);

    ActionInfo.GenerateColor();

    return ActionInfo;
}

bool UEVGameInstance::GetStoredNotificationSettings(FEVPopUpSettingsInfo& OutSettings) const
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

bool UEVGameInstance::ConsumePendingNotificationWord(FString& OutWord) const
{
    OutWord.Empty();
    return DeviceService && DeviceService->ConsumePendingNotificationWord(OutWord);
}

bool UEVGameInstance::AreNotificationsEnabled() const
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot check notifications: DeviceService is nullptr."));

        return false;
    }

    return DeviceService->AreNotificationsEnabled();
}

bool UEVGameInstance::HasRequestedNotificationPermission() const
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot check notification permission history: DeviceService is nullptr."));

        return false;
    }

    return DeviceService->HasRequestedNotificationPermission();
}

bool UEVGameInstance::RequestNotificationPermission()
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot request notification permission: DeviceService is nullptr."));

        return false;
    }

    return DeviceService->RequestNotificationPermission();
}

void UEVGameInstance::OpenNotificationSettings()
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot open notification settings: DeviceService is nullptr."));

        return;
    }

    DeviceService->OpenNotificationSettings();
}

void UEVGameInstance::TestDeviceAlarm()
{
    if (!DeviceService)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot open notification settings: DeviceService is nullptr."));

        return;
    }

    DeviceService->TestAlarm();
}

void UEVGameInstance::HandleNotificationPermissionResult(const bool bGranted)
{
    NotificationPermissionResultDelegate.Broadcast(bGranted);
}
int32 UEVGameInstance::GetVocabularyEntryCountByCriteria(const FString& SearchPrefix,
                                                         const FEVVocabularyQueryCriteria& Criteria) const
{
    if (!VocabularyStorageService)
    {
        UE_LOG(LogTemp, Error, TEXT("GetVocabularyEntryCountByCriteria: VocabularyStorageService is null"));
        return 0;
    }
    return VocabularyStorageService->GetVocabularyEntryCountByCriteria(SearchPrefix, Criteria);
}

bool UEVGameInstance::GetVocabularyEntriesPageByCriteria(TArray<FVocabularyEntry>& OutVocabularyEntries,
                                                         const FString& SearchPrefix,
                                                         const FEVVocabularyQueryCriteria& Criteria, const int32 Limit,
                                                         const int32 Offset) const
{
    OutVocabularyEntries.Reset();
    if (!VocabularyStorageService || Limit <= 0 || Offset < 0)
    {
        return false;
    }

    OutVocabularyEntries =
        VocabularyStorageService->GetVocabularyEntriesPageByCriteria(SearchPrefix, Criteria, Limit, Offset);
    return true;
}

void UEVGameInstance::SetActiveVocabularyQueryCriteria(const FEVVocabularyQueryCriteria& Criteria)
{
    ActiveVocabularyQueryCriteria = Criteria;
    ActiveVocabularyQueryCriteria.Normalize();
}

const FEVVocabularyQueryCriteria& UEVGameInstance::GetActiveVocabularyQueryCriteria() const
{
    return ActiveVocabularyQueryCriteria;
}
