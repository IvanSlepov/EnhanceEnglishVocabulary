// Fill out your copyright notice in the Description page of Project Settings.

#include "EVGameInstance.h"

#include "EVVocabularyStorageService.h"
#include "EVWordSearchService.h"
#include "EVConnectivityService.h"
#include "EVDeviceService.h"

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

    FileExchangeCoordinator = NewObject<UEVFileExchangeApplicationCoordinator>(this);

    NotificationCoordinator = NewObject<UEVNotificationApplicationCoordinator>(this);

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
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DeviceService is null"));
    }

    if (FileExchangeCoordinator)
    {
        FileExchangeCoordinator->Initialize(VocabularyStorageService, DeviceService);
    }
    if (NotificationCoordinator)
    {
        NotificationCoordinator->Initialize(VocabularyStorageService, DeviceService);
    }
}

void UEVGameInstance::Shutdown()
{
    SaveVocabularyLanguagePreferences();
    if (ApplicationWillEnterBackgroundHandle.IsValid())
    {
        FCoreDelegates::ApplicationWillEnterBackgroundDelegate.Remove(ApplicationWillEnterBackgroundHandle);
    }

    if (FileExchangeCoordinator)
    {
        FileExchangeCoordinator->Shutdown();
    }
    if (NotificationCoordinator)
    {
        NotificationCoordinator->Shutdown();
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
    return NotificationCoordinator && NotificationCoordinator->ApplySettings(PopUpSettings);
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
        Outcome.bExists = true;
        Outcome.Message = ErrorMessage;
        Outcome.Result = GetVocabularyRecordByWord(Request.Word, Outcome.Record)
                             ? EEVApplicationOperationResult::Succeeded
                             : EEVApplicationOperationResult::Failed;
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
            UE_LOG(LogTemp, Error, TEXT("Failed to load structured vocabulary record for word: %s"), *Entry.Word);
            continue;
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
    if (FileExchangeCoordinator)
    {
        return FileExchangeCoordinator->HandleFileOperationRequested(FileOperationInfo);
    }

    FEVRequestedActionInfo ActionInfo;
    ActionInfo.Source = EEVRequestedActionSource::ImportExport;
    ActionInfo.Status = EEVRequestedActionStatus::Failed;
    ActionInfo.Message = FText::FromString(TEXT("File exchange capability is unavailable."));
    ActionInfo.GenerateColor();
    return ActionInfo;
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

FEVFileOperationCompletedFromGameInstance& UEVGameInstance::OnFileOperationCompleted()
{
    if (FileExchangeCoordinator)
    {
        return FileExchangeCoordinator->OnFileOperationCompleted();
    }

    static FEVFileOperationCompletedFromGameInstance UnavailableDelegate;
    return UnavailableDelegate;
}

FEVImportFilePickCompleted& UEVGameInstance::OnImportFilePickCompleted()
{
    if (FileExchangeCoordinator)
    {
        return FileExchangeCoordinator->OnImportFilePickCompleted();
    }

    static FEVImportFilePickCompleted UnavailableDelegate;
    return UnavailableDelegate;
}

bool UEVGameInstance::GetStoredNotificationSettings(FEVPopUpSettingsInfo& OutSettings) const
{
    return NotificationCoordinator && NotificationCoordinator->GetStoredSettings(OutSettings);
}

bool UEVGameInstance::ConsumePendingNotificationWord(FString& OutWord) const
{
    return NotificationCoordinator && NotificationCoordinator->ConsumePendingNotificationWord(OutWord);
}

bool UEVGameInstance::AreNotificationsEnabled() const
{
    return NotificationCoordinator && NotificationCoordinator->AreNotificationsEnabled();
}

bool UEVGameInstance::HasRequestedNotificationPermission() const
{
    return NotificationCoordinator && NotificationCoordinator->HasRequestedNotificationPermission();
}

bool UEVGameInstance::RequestNotificationPermission()
{
    return NotificationCoordinator && NotificationCoordinator->RequestNotificationPermission();
}

void UEVGameInstance::OpenNotificationSettings()
{
    if (NotificationCoordinator)
    {
        NotificationCoordinator->OpenNotificationSettings();
    }
}

void UEVGameInstance::TestDeviceAlarm()
{
    if (NotificationCoordinator)
    {
        NotificationCoordinator->TestDeviceAlarm();
    }
}

FEVNotificationPermissionResultFromGameInstance& UEVGameInstance::OnNotificationPermissionResult()
{
    if (NotificationCoordinator)
    {
        return NotificationCoordinator->OnNotificationPermissionResult();
    }

    static FEVNotificationPermissionResultFromGameInstance UnavailableDelegate;
    return UnavailableDelegate;
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
