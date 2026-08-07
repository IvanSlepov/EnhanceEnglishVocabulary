#pragma once

#include "CoreMinimal.h"

class ENHANCEVOCABULARYSTORAGE_API FEVVocabularySqlQueries
{
public:
    static TArray<FString> GetCreateNormalizedSchemaQueries();
    static FString GetSelectCompatibilityColumnsQuery(const FString& WhereClause = FString(),
                                                      const FString& OrderAndLimitClause = FString());
    static FString GetVocabularyEntryByWordQuery();
    static FString GetSelectImportExportColumnsQuery();
    static FString GetSelectVocabularyEntriesQuery();
    static FString GetSelectVocabularyEntriesPageQuery();
    static FString GetSelectVocabularyEntriesPageByPrefixQuery();
    static FString GetVocabularyEntryCountByCriteriaQuery(int32 PartOfSpeechCount, bool bHasPrefix);
    static FString GetSelectVocabularyEntriesPageByCriteriaQuery(int32 PartOfSpeechCount, bool bHasPrefix);
    static FString GetRandomlySelectedWordQuery();

    static constexpr const TCHAR* DeleteVocabularyEntry =
        TEXT("DELETE FROM VocabularyEntries WHERE NormalizedWord = ?;");
    static constexpr const TCHAR* CountVocabularyEntries = TEXT("SELECT COUNT(*) FROM VocabularyEntries;");
    static constexpr const TCHAR* WordExists =
        TEXT("SELECT 1 FROM VocabularyEntries WHERE NormalizedWord = ? LIMIT 1;");
    static constexpr const TCHAR* GetVocabularyEntryCountByPrefixQuery =
        TEXT("SELECT COUNT(*) FROM VocabularyEntries WHERE NormalizedWord LIKE ? COLLATE NOCASE;");
};
