// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EVConnectionTypesAndEnums.h"
#include "EVWebProviderTypes.h"
#include "EVFileExchangeTypes.h"
#include "EVRequestedActionTypes.h"
#include "EVFileExchangeDefaults.h"
#include "EVPopUpSettingsTypes.h"
#include "EVVocabularyFilterTypes.h"
#include "EVVocabularyLanguageTypes.h"

#include "EVGameInstance.generated.h"

class UEVVocabularyStorageService;
class UEVWordSearchService;
class UEVConnectivityService;
class UEVDeviceService;

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEVConnectionStateChanged, EEVConnectionState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEVWordSearchCompletedFromEVGameInstance, const FWordSearchResult&, Result);
DECLARE_MULTICAST_DELEGATE_OneParam(FEVFileOperationCompletedFromGameInstance, const FEVRequestedActionInfo&);
DECLARE_MULTICAST_DELEGATE_OneParam(FEVImportFilePickCompleted, const FEVFileExchangeResultInfo&);
DECLARE_MULTICAST_DELEGATE_OneParam(FEVNotificationPermissionResultFromGameInstance, bool /* bGranted */);

UENUM()
enum class EEVVocabularyStorageServiceResult : uint8
{
    WordExists,
    WordDoesNotExist,
    DatabaseError,
    VocabularyStorageInstanceError,
    Empty
};

enum class EEVPendingFileSavePurpose : uint8
{
    None,
    DownloadTemplate,
    ExportDatabase,
    ImportValidationReport
};

UCLASS()
class ENHANCEVOCABULARY_API UEVGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TObjectPtr<UEVVocabularyStorageService> VocabularyStorageService;

    UPROPERTY()
    TObjectPtr<UEVWordSearchService> WordSearchService;

    UPROPERTY()
    TObjectPtr<UEVConnectivityService> ConnectivityService;

    UPROPERTY()
    TObjectPtr<UEVDeviceService> DeviceService;

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    EEVVocabularyStorageServiceResult DoesWordExist(const FString& Word, FText& OutErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    bool SaveVocabularyEntry(const FWordSearchResult& WordSearchResult);

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    bool UpdateVocabularyEntry(const FVocabularyEntry& Entry, FVocabularyEntry& OutEntry);

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    bool UpdateVocabularyRecord(const FEVVocabularyRecord& Record, FEVVocabularyRecord& OutRecord);

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    bool DeleteVocabularyEntry(const FVocabularyEntry& Entry);

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    bool GetVocabularyEntryByWord(const FString& Word, FVocabularyEntry& OutEntry) const;

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    bool GetVocabularyRecordByWord(const FString& Word, FEVVocabularyRecord& OutRecord) const;

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    int32 GetVocabularyEntryCount() const;

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    bool GetVocabularyEntriesPage(TArray<FVocabularyEntry>& OutVocabularyEntries, int32 Limit, int32 Offset) const;

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    bool GetVocabularyEntries(TArray<FVocabularyEntry>& OutVocabularyEntries, int32 EntryNumber = 5);

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    int32 GetVocabularyEntryCountByPrefix(const FString& SearchPrefix) const;

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    bool GetVocabularyEntriesPageByPrefix(TArray<FVocabularyEntry>& OutVocabularyEntries, const FString& SearchPrefix,
                                          int32 Limit, int32 Offset) const;

    int32 GetVocabularyEntryCountByCriteria(const FString& SearchPrefix,
                                            const FEVVocabularyQueryCriteria& Criteria) const;

    bool GetVocabularyEntriesPageByCriteria(TArray<FVocabularyEntry>& OutVocabularyEntries, const FString& SearchPrefix,
                                            const FEVVocabularyQueryCriteria& Criteria, int32 Limit,
                                            int32 Offset) const;

    void SetActiveVocabularyQueryCriteria(const FEVVocabularyQueryCriteria& Criteria);
    const FEVVocabularyQueryCriteria& GetActiveVocabularyQueryCriteria() const;

    const FEVVocabularyLanguagePreferences& GetVocabularyLanguagePreferences() const;
    bool SetVocabularyLanguagePreferences(const FEVVocabularyLanguagePreferences& Preferences);

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Storage")
    bool GetRandomlySelectedWord(FString& OutWord);

    UFUNCTION(BlueprintCallable, Category = "Pop-up settings")
    bool HandlePopUpIntervalSelected(const FEVPopUpSettingsInfo& PopUpSettings);

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Fake/Debugging Search")
    FWordSearchResult SearchWordFake(const FString& Word);

    UFUNCTION(BlueprintCallable, Category = "Vocabulary Online Search")
    void SearchWordOnline(const FString& Word, EEVWebProvider DefinitionUsageProvider,
                          EEVWebProvider TranslationProvider);

    UFUNCTION(BlueprintCallable, Category = "File Exchange")
    FEVRequestedActionInfo HandleFileOperationRequested(const FEVFileOperationInfo& FileOperationInfo);

    // Connection functions
    EEVConnectionState GetConnectionState() const;

    /*Events*/
    UPROPERTY(BlueprintAssignable)
    FEVConnectionStateChanged OnConnectionStateChanged;

    UPROPERTY(BlueprintAssignable)
    FEVWordSearchCompletedFromEVGameInstance OnEVWordSearchCompletedFromEVGameInstance;

    FEVFileOperationCompletedFromGameInstance& OnFileOperationCompleted();

    FEVImportFilePickCompleted& OnImportFilePickCompleted()
    {
        return ImportFilePickCompletedDelegate;
    }

    FEVNotificationPermissionResultFromGameInstance& OnNotificationPermissionResult()
    {
        return NotificationPermissionResultDelegate;
    }

public:
    /*Notifications(Pop-ups) related functions*/
    bool AreNotificationsEnabled() const;

    bool GetStoredNotificationSettings(FEVPopUpSettingsInfo& OutSettings) const;
    bool ConsumePendingNotificationWord(FString& OutWord) const;

    bool HasRequestedNotificationPermission() const;

    bool RequestNotificationPermission();

    void OpenNotificationSettings();

    void TestDeviceAlarm();

protected:
    virtual void Init() override;
    virtual void Shutdown() override;

private:
    UFUNCTION()
    void HandleConnectionStateChanged(EEVConnectionState NewState);

    UFUNCTION()
    void HandleEVWordSearchCompletedFromEVGameInstance(const FWordSearchResult& SearchWordResultPassedByGameInstance);

    UFUNCTION()
    void HandlePopUpTimerExpired();

    // We need to assign local ENUM var (Cause unassigned enum type var takes the very first entry from that enum)
    // and we need to assign smth that differ from the Offline in this case, to address the issue when
    // the app is launched with already disabled Internet access but the corresponding error message never appears
    // since the even responsible for that action never fires at launch due to local var being Offline and
    // the connectivity var being set to Offline and that makes the check in the corresponding Handler to return
    // immediately
    EEVConnectionState EVConnectionState = EEVConnectionState::Connecting;
    EEVPendingFileSavePurpose PendingFileSavePurpose = EEVPendingFileSavePurpose::None;

    void HandleFileSaved(const FEVFileExchangeResultInfo& ResultInfo);

    // HandleImportFilePicked related funcs
    void HandleImportFilePicked(const FEVFileExchangeResultInfo& ResultInfo, const TArray<uint8>& Bytes);

    void CompleteImportFileOperation(const FEVFileExchangeResultInfo& ResultInfo);

    bool TrySaveImportValidationReport(FEVFileExchangeResultInfo ValidationResult,
                                       const TArray<uint8>& ValidationReportBytes);

    FEVFileExchangeResultInfo ExecuteImportDatabaseOperation(const TArray<FEVVocabularyRecord>& ValidatedRecords);

    void PopulateImportResultFileInfo(FEVFileExchangeResultInfo& ResultInfo,
                                      const FEVFileExchangeResultInfo& PickResult, int32 ByteCount) const;
    // --- end of the HandleImportFilePicked funcs

    FEVRequestedActionInfo HandleDownloadTemplateRequested(const FEVFileOperationInfo& FileOperationInfo);

    FEVRequestedActionInfo HandleExportDBRequested(const FEVFileOperationInfo& FileOperationInfo);

    FEVRequestedActionInfo HandleImportDBOverwriteRequested(const FEVFileOperationInfo& FileOperationInfo);

    FEVRequestedActionInfo HandleImportDBAppendRequested(const FEVFileOperationInfo& FileOperationInfo);

    FEVRequestedActionInfo
    ConvertFileExchangeResultToRequestedAction(const FEVFileExchangeResultInfo& ResultInfo) const;

    FEVFileOperationCompletedFromGameInstance FileOperationCompletedDelegate;
    FEVImportFilePickCompleted ImportFilePickCompletedDelegate;

    FEVFileOperationInfo PendingImportFileOperationInfo;
    FEVFileExchangeResultInfo PendingImportValidationResult;

    FEVPopUpSettingsInfo CurrentPopUpSettings;

    UPROPERTY(Transient)
    FEVVocabularyQueryCriteria ActiveVocabularyQueryCriteria;

    FEVNotificationPermissionResultFromGameInstance NotificationPermissionResultDelegate;

    UPROPERTY(Transient)
    FEVVocabularyLanguagePreferences VocabularyLanguagePreferences;

    FDelegateHandle ApplicationWillEnterBackgroundHandle;

    void LoadVocabularyLanguagePreferences();
    void SaveVocabularyLanguagePreferences() const;
    void HandleApplicationWillEnterBackground();

    void HandleNotificationPermissionResult(const bool bGranted);
};
