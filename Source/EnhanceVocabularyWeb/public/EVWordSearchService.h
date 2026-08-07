#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVVocabularyTypes.h"
#include "EVVocabularyLanguageTypes.h"
#include "EVWebProviderTypes.h"
#include "EVWordSearchService.generated.h"

class UEVHttpService;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEVWordSearchCompleted, const FWordSearchResult&, Result);

UCLASS()
class ENHANCEVOCABULARYWEB_API UEVWordSearchService : public UObject
{
    GENERATED_BODY()

public:
    void Initialize();

    UFUNCTION(BlueprintCallable, Category = "Word Search")
    FWordSearchResult SearchWordFake(const FString& Word);

    UFUNCTION(BlueprintCallable, Category = "Word Search")
    void SearchWordOnline(const FString& Word, EEVWebProvider DefinitionUsageProvider,
                          EEVWebProvider TranslationProvider, EEVVocabularyDBContext DatabaseContext,
                          const TArray<EEVVocabularyTranslationLanguage>& TranslationLanguages);

    UPROPERTY(BlueprintAssignable)
    FEVWordSearchCompleted OnEVWordSearchCompleted;

private:
    void SendDictionaryRequest(const FString& Word, EEVWebProvider DefinitionUsageProvider);
    void SendTranslationRequest(const FString& Word, EEVVocabularyDBContext DatabaseContext,
                                EEVVocabularyTranslationLanguage TranslateTo, EEVWebProvider TranslationProvider);

    void HandleDictionaryResponse(bool bSuccess, int32 ResponseCode, const FString& ResponseBody);
    void HandleTranslationResponse(bool bSuccess, int32 ResponseCode, const FString& ResponseBody,
                                   EEVVocabularyTranslationLanguage TargetLanguage);

    void ResetPendingSearch(const FString& Word);
    void TryCompleteSearch();
    void PopulateLegacySearchResult();

private:
    UPROPERTY()
    TObjectPtr<UEVHttpService> HttpService;

    FWordSearchResult PendingResult;
    FEVVocabularyRecord PendingRecord;

    bool bDictionaryCompleted = false;
    bool bDictionarySucceeded = false;
    int32 PendingTranslationRequestCount = 0;

    FString CurrentSearchWord;
};
