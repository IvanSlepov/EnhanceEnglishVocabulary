#include "EVVocabularyInteractionCoordinator.h"

#include "EVApplicationFeatureTypes.h"
#include "EVEntryDetailsApplicationPort.h"
#include "EVFeatureNavigationApplicationPort.h"
#include "EVGameInstance.h"
#include "EVVocabularyFilterApplicationPort.h"
#include "EVVocabularyLanguageTypes.h"
#include "EVVocabularyLegacyCompatibility.h"
#include "EVVocabularyLibraryApplicationPort.h"
#include "EVWidgetCommonEvents.h"

void UEVVocabularyInteractionCoordinator::Initialize(UEVGameInstance* InGameInstance,
                                                     IEVEntryDetailsApplicationPort* InEntryDetailsPort,
                                                     IEVVocabularyFilterApplicationPort* InFilterPort,
                                                     IEVFeatureNavigationApplicationPort* InNavigationPort,
                                                     IEVVocabularyLibraryApplicationPort* InLibraryPort,
                                                     IEVWidgetCommonEvents* InLegacyEvents)
{
    GameInstance = InGameInstance;
    UpdatePresentationPorts(InEntryDetailsPort, InFilterPort, InNavigationPort, InLibraryPort, InLegacyEvents);
}

void UEVVocabularyInteractionCoordinator::UpdatePresentationPorts(IEVEntryDetailsApplicationPort* InEntryDetailsPort,
                                                                  IEVVocabularyFilterApplicationPort* InFilterPort,
                                                                  IEVFeatureNavigationApplicationPort* InNavigationPort,
                                                                  IEVVocabularyLibraryApplicationPort* InLibraryPort,
                                                                  IEVWidgetCommonEvents* InLegacyEvents)
{
    EntryDetailsPort = InEntryDetailsPort;
    FilterPort = InFilterPort;
    NavigationPort = InNavigationPort;
    LibraryPort = InLibraryPort;
    LegacyEvents = InLegacyEvents;
}

void UEVVocabularyInteractionCoordinator::PresentEntryDetails(const FEVVocabularyRecord& Record)
{
    CurrentRecord = Record;
    PendingRecord = FEVVocabularyRecord{};
    CurrentLegacyEntryAction.ActionType = EEVWordEntryActionType::ViewEntry;
    CurrentLegacyEntryAction.EntryInfo = EVVocabularyLegacyCompatibility::FlattenRecord(Record);
    if (EntryDetailsPort)
    {
        EntryDetailsPort->PresentEntryDetails(CurrentRecord);
    }
}

void UEVVocabularyInteractionCoordinator::PresentLegacyEntryDetails(const FEVWordEntryActionInfo& EntryAction)
{
    CurrentLegacyEntryAction = EntryAction;
    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot load Entry Details: GameInstance is unavailable."));
        return;
    }

    FEVVocabularyRecord Record;
    const FString RecordKey = EntryAction.EntryInfo.NormalizedWord.IsEmpty() ? EntryAction.EntryInfo.Word
                                                                             : EntryAction.EntryInfo.NormalizedWord;
    if (!GameInstance->GetVocabularyRecordByWord(RecordKey, Record))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load structured vocabulary record for Entry Details: %s"),
               *EntryAction.EntryInfo.Word);
        return;
    }
    PresentEntryDetails(Record);
}

void UEVVocabularyInteractionCoordinator::RequestEntryDetailsClose()
{
    CurrentLegacyEntryAction.ActionType = EEVWordEntryActionType::CloseEntry;
    ConfirmationRequested.Broadcast(EEVConfirmationDialogType::ExitViewWord, EEVWordEntryActionType::CloseEntry);
}

void UEVVocabularyInteractionCoordinator::RequestEntryDetailsSave(const FEVVocabularyRecord& Record)
{
    PendingRecord = Record;
    CurrentLegacyEntryAction.ActionType = EEVWordEntryActionType::SaveEditedEntry;
    ConfirmationRequested.Broadcast(EEVConfirmationDialogType::EditWord, EEVWordEntryActionType::SaveEditedEntry);
}

void UEVVocabularyInteractionCoordinator::RequestEntryDetailsDelete()
{
    CurrentLegacyEntryAction.ActionType = EEVWordEntryActionType::DeleteEntry;
    ConfirmationRequested.Broadcast(EEVConfirmationDialogType::DeleteWord, EEVWordEntryActionType::DeleteEntry);
}

bool UEVVocabularyInteractionCoordinator::ResolveConfirmation(const EEVConfirmationDialogType DialogType,
                                                              const bool bConfirmed)
{
    if (DialogType == EEVConfirmationDialogType::ReviewExistingRelatedWord ||
        DialogType == EEVConfirmationDialogType::AddMissingRelatedWord)
    {
        if (bConfirmed)
        {
            const bool bReview = DialogType == EEVConfirmationDialogType::ReviewExistingRelatedWord;
            const FString Word = PendingValueAction.Relation.RelatedWord;
            if (NavigationPort)
            {
                FEVFeatureNavigationRequest Request;
                Request.FeatureId = bReview ? EVApplicationFeature::ReviewWords : EVApplicationFeature::AddWord;
                Request.Context = Word;
                NavigationPort->ApplyFeatureNavigation(Request);
            }
            else if (LegacyEvents)
            {
                if (bReview)
                {
                    LegacyEvents->HandleOpenReviewWordsForNotification(Word);
                }
                else
                {
                    LegacyEvents->HandleOpenAddWordWithWord(Word);
                }
            }
        }
        PendingValueAction = FEVVocabularyValueActionRequest{};
        return true;
    }

    if (DialogType == EEVConfirmationDialogType::UnsupportedTranslationLanguage ||
        DialogType == EEVConfirmationDialogType::CreateTranslationLanguageContext ||
        DialogType == EEVConfirmationDialogType::ReviewExistingTranslationWord ||
        DialogType == EEVConfirmationDialogType::AddMissingTranslationWord)
    {
        // Cross-context creation and navigation are future attachment points; existing behavior is intentionally no-op.
        PendingValueAction = FEVVocabularyValueActionRequest{};
        return true;
    }

    if (DialogType != EEVConfirmationDialogType::ExitViewWord && DialogType != EEVConfirmationDialogType::EditWord &&
        DialogType != EEVConfirmationDialogType::DeleteWord)
    {
        return false;
    }

    if (!bConfirmed)
    {
        if (CurrentLegacyEntryAction.ActionType == EEVWordEntryActionType::SaveEditedEntry)
        {
            PendingRecord = FEVVocabularyRecord{};
            if (EntryDetailsPort)
            {
                EntryDetailsPort->PresentEntryDetails(CurrentRecord);
            }
        }
        return true;
    }

    switch (CurrentLegacyEntryAction.ActionType)
    {
    case EEVWordEntryActionType::CloseEntry:
        if (EntryDetailsPort)
        {
            EntryDetailsPort->DismissEntryDetails();
        }
        break;
    case EEVWordEntryActionType::SaveEditedEntry:
        ProcessConfirmedWordUpdate();
        break;
    case EEVWordEntryActionType::DeleteEntry:
        ProcessConfirmedWordDelete();
        break;
    default:
        break;
    }
    return true;
}

void UEVVocabularyInteractionCoordinator::ProcessConfirmedWordUpdate()
{
    LoadingChanged.Broadcast(true);
    FEVVocabularyRecord UpdatedRecord;
    const bool bUpdated = GameInstance && GameInstance->UpdateVocabularyRecord(PendingRecord, UpdatedRecord);
    LoadingChanged.Broadcast(false);

    FEVRequestedActionInfo StatusInfo;
    StatusInfo.Source = EEVRequestedActionSource::ReviewWords;
    StatusInfo.Type = EEVRequestedActionType::EditWord;
    StatusInfo.Status = bUpdated ? EEVRequestedActionStatus::Saved : EEVRequestedActionStatus::Failed;
    StatusInfo.GenerateMessage();
    StatusInfo.GenerateColor();
    StatusRequested.Broadcast(StatusInfo);
    if (!bUpdated)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to update structured vocabulary record."));
        return;
    }

    CurrentRecord = UpdatedRecord;
    PendingRecord = FEVVocabularyRecord{};
    FVocabularyEntry UpdatedLegacyEntry;
    if (!GameInstance->GetVocabularyEntryByWord(UpdatedRecord.NormalizedWord, UpdatedLegacyEntry))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load compatibility entry after structured update: %s"),
               *UpdatedRecord.Word);
        return;
    }

    CurrentLegacyEntryAction.ActionType = EEVWordEntryActionType::SaveEditedEntry;
    CurrentLegacyEntryAction.EntryInfo = UpdatedLegacyEntry;
    if (EntryDetailsPort)
    {
        EntryDetailsPort->PresentEntryDetails(CurrentRecord);
    }
    ApplyVocabularyChange(EEVVocabularyChangeType::Updated, UpdatedLegacyEntry);
}

void UEVVocabularyInteractionCoordinator::ProcessConfirmedWordDelete()
{
    LoadingChanged.Broadcast(true);
    const bool bDeleted = GameInstance && GameInstance->DeleteVocabularyEntry(CurrentLegacyEntryAction.EntryInfo);
    LoadingChanged.Broadcast(false);

    FEVRequestedActionInfo StatusInfo;
    StatusInfo.Source = EEVRequestedActionSource::ReviewWords;
    StatusInfo.Type = EEVRequestedActionType::DeleteWord;
    StatusInfo.Status = bDeleted ? EEVRequestedActionStatus::Done : EEVRequestedActionStatus::Failed;
    StatusInfo.GenerateMessage();
    StatusInfo.GenerateColor();
    StatusRequested.Broadcast(StatusInfo);
    if (!bDeleted)
    {
        return;
    }

    CurrentLegacyEntryAction.ActionType = EEVWordEntryActionType::DeleteEntry;
    ApplyVocabularyChange(EEVVocabularyChangeType::Removed, CurrentLegacyEntryAction.EntryInfo);
    if (EntryDetailsPort)
    {
        EntryDetailsPort->DismissEntryDetails();
    }
}

void UEVVocabularyInteractionCoordinator::ApplyVocabularyChange(const EEVVocabularyChangeType ChangeType,
                                                                const FVocabularyEntry& Entry)
{
    if (LibraryPort)
    {
        FEVVocabularyChangeInfo ChangeInfo;
        ChangeInfo.ChangeId = FGuid::NewGuid();
        ChangeInfo.OriginId = FName(TEXT("EntryDetails"));
        ChangeInfo.ChangeType = ChangeType;
        ChangeInfo.AffectedNormalizedWords.Add(Entry.NormalizedWord.IsEmpty() ? Entry.Word : Entry.NormalizedWord);
        LibraryPort->ApplyVocabularyChanged(ChangeInfo);
    }
    else if (LegacyEvents)
    {
        LegacyEvents->HandleWordEntryChanged(CurrentLegacyEntryAction);
    }
}

void UEVVocabularyInteractionCoordinator::RequestVocabularyValueAction(const FEVVocabularyValueActionRequest& Request)
{
    PendingValueAction = Request;
    if (Request.ActionType == EEVVocabularyValueActionType::Synonym ||
        Request.ActionType == EEVVocabularyValueActionType::Antonym)
    {
        RequestRelationAction(Request);
    }
    else if (Request.ActionType == EEVVocabularyValueActionType::Translation)
    {
        RequestTranslationAction(Request);
    }
}

void UEVVocabularyInteractionCoordinator::RequestRelationAction(const FEVVocabularyValueActionRequest& Request)
{
    if (!GameInstance)
    {
        return;
    }

    FString LookupWord = Request.Relation.NormalizedRelatedWord.TrimStartAndEnd();
    if (LookupWord.IsEmpty())
    {
        LookupWord = Request.Relation.RelatedWord.TrimStartAndEnd();
        LookupWord.ToLowerInline();
    }
    FVocabularyEntry ExistingEntry;
    const bool bExists = GameInstance->GetVocabularyEntryByWord(LookupWord, ExistingEntry);
    ConfirmationRequested.Broadcast(bExists ? EEVConfirmationDialogType::ReviewExistingRelatedWord
                                            : EEVConfirmationDialogType::AddMissingRelatedWord,
                                    EEVWordEntryActionType::Unknown);
}

void UEVVocabularyInteractionCoordinator::RequestTranslationAction(const FEVVocabularyValueActionRequest& Request)
{
    const EEVVocabularyLanguageSupportState SupportState =
        ResolveTranslationLanguageSupport(Request.Translation.TargetLanguage);
    if (SupportState == EEVVocabularyLanguageSupportState::Unsupported ||
        SupportState == EEVVocabularyLanguageSupportState::Unknown)
    {
        ConfirmationRequested.Broadcast(EEVConfirmationDialogType::UnsupportedTranslationLanguage,
                                        EEVWordEntryActionType::Unknown);
        return;
    }
    if (SupportState == EEVVocabularyLanguageSupportState::SupportedWithoutContext)
    {
        ConfirmationRequested.Broadcast(EEVConfirmationDialogType::CreateTranslationLanguageContext,
                                        EEVWordEntryActionType::Unknown);
        return;
    }

    ConfirmationRequested.Broadcast(DoesTranslationWordExistInTargetContext(Request.Translation)
                                        ? EEVConfirmationDialogType::ReviewExistingTranslationWord
                                        : EEVConfirmationDialogType::AddMissingTranslationWord,
                                    EEVWordEntryActionType::Unknown);
}

bool UEVVocabularyInteractionCoordinator::DoesTranslationWordExistInTargetContext(
    const FEVVocabularyTranslation& Translation) const
{
    if (!GameInstance)
    {
        return false;
    }

    EEVVocabularyTranslationLanguage TranslationLanguage = EEVVocabularyTranslationLanguage::None;
    EEVVocabularyDBContext TargetContext = EEVVocabularyDBContext::None;
    if (!EVVocabularyLanguage::TryParseTranslationLanguage(Translation.TargetLanguage, TranslationLanguage) ||
        !EVVocabularyLanguage::TryResolveDatabaseContextForTranslation(TranslationLanguage, TargetContext) ||
        TargetContext != GameInstance->GetVocabularyLanguagePreferences().DatabaseContext)
    {
        return false;
    }

    FVocabularyEntry ExistingEntry;
    return GameInstance->GetVocabularyEntryByWord(Translation.TranslationText, ExistingEntry);
}

EEVVocabularyLanguageSupportState
UEVVocabularyInteractionCoordinator::ResolveTranslationLanguageSupport(const FString& LanguageCode) const
{
    EEVVocabularyTranslationLanguage TranslationLanguage = EEVVocabularyTranslationLanguage::None;
    if (!EVVocabularyLanguage::TryParseTranslationLanguage(LanguageCode, TranslationLanguage))
    {
        return EEVVocabularyLanguageSupportState::Unsupported;
    }
    EEVVocabularyDBContext TargetContext = EEVVocabularyDBContext::None;
    return EVVocabularyLanguage::TryResolveDatabaseContextForTranslation(TranslationLanguage, TargetContext)
               ? EEVVocabularyLanguageSupportState::SupportedWithContext
               : EEVVocabularyLanguageSupportState::SupportedWithoutContext;
}

FString UEVVocabularyInteractionCoordinator::GetConfirmationSubject(const EEVConfirmationDialogType DialogType) const
{
    if (DialogType == EEVConfirmationDialogType::ReviewExistingRelatedWord ||
        DialogType == EEVConfirmationDialogType::AddMissingRelatedWord)
    {
        return PendingValueAction.Relation.RelatedWord;
    }
    if (DialogType == EEVConfirmationDialogType::UnsupportedTranslationLanguage ||
        DialogType == EEVConfirmationDialogType::CreateTranslationLanguageContext)
    {
        EEVVocabularyTranslationLanguage Language = EEVVocabularyTranslationLanguage::None;
        return EVVocabularyLanguage::TryParseTranslationLanguage(PendingValueAction.Translation.TargetLanguage,
                                                                 Language)
                   ? EVVocabularyLanguage::GetTranslationLanguageDisplayText(Language).ToString()
                   : PendingValueAction.Translation.TargetLanguage;
    }
    if (DialogType == EEVConfirmationDialogType::ReviewExistingTranslationWord ||
        DialogType == EEVConfirmationDialogType::AddMissingTranslationWord)
    {
        return PendingValueAction.Translation.TranslationText;
    }
    return FString();
}

void UEVVocabularyInteractionCoordinator::RequestVocabularyFilters()
{
    if (!GameInstance || !FilterPort)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot present vocabulary filters: an application attachment is missing."));
        return;
    }
    FilterPort->PresentVocabularyFilters(GameInstance->GetActiveVocabularyQueryCriteria());
}

void UEVVocabularyInteractionCoordinator::ApplyVocabularyFilters(const FEVVocabularyQueryCriteria& Criteria)
{
    if (GameInstance)
    {
        GameInstance->SetActiveVocabularyQueryCriteria(Criteria);
    }
    if (FilterPort)
    {
        FilterPort->ApplyVocabularyFilterCriteria(Criteria);
    }
}
