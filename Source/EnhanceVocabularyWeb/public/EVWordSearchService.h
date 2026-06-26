#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EVVocabularyTypes.h"
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
    void SearchWordOnline(const FString& Word);

    UPROPERTY(BlueprintAssignable)
    FEVWordSearchCompleted OnEVWordSearchCompleted;

private:
    void SendDictionaryRequest(const FString& Word);
    void SendTranslationRequest(const FString& Word, const FString& TranslateTo);

    void HandleDictionaryResponse(bool bSuccess, int32 ResponseCode, const FString& ResponseBody);
    void HandleTranslationRuResponse(bool bSuccess, int32 ResponseCode, const FString& ResponseBody);
    void HandleTranslationUkResponse(bool bSuccess, int32 ResponseCode, const FString& ResponseBody);

    void ResetPendingSearch(const FString& Word);
    void TryCompleteSearch();

private:
    UPROPERTY()
    TObjectPtr<UEVHttpService> HttpService;

    FWordSearchResult PendingResult;

    bool bDictionaryCompleted = false;
    bool bTranslationRuCompleted = false;
    bool bTranslationUkCompleted = false;

    FString CurrentSearchWord;
};