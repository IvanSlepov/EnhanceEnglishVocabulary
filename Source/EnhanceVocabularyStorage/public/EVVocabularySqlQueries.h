#pragma once

#include "CoreMinimal.h"

class ENHANCEVOCABULARYSTORAGE_API FEVVocabularySqlQueries
{
public:
    static FString GetCreateVocabularyTableQuery();
    static FString GetInsertVocabularyEntryStrictQuery();
    static FString GetEditVocabularyEntryQuery();
    static FString GetVocabularyEntryByWordQuery();
    static FString GetSelectImportExportColumnsQuery();
    static FString GetSelectVocabularyEntriesQuery();
    static FString GetSelectVocabularyEntriesPageQuery();
    static FString GetSelectVocabularyEntriesPageByPrefixQuery();

    static constexpr const TCHAR* DeleteVocabularyEntry = TEXT("DELETE FROM VocabularyEntries WHERE Word = ?;");

    static constexpr const TCHAR* CountVocabularyEntries = TEXT("SELECT COUNT(*) FROM VocabularyEntries;");

    static constexpr const TCHAR* WordExists = TEXT("SELECT 1 FROM VocabularyEntries WHERE Word = ? LIMIT 1;");

    static constexpr const TCHAR* GetVocabularyEntryCountByPrefixQuery =
        TEXT("SELECT COUNT(*) FROM VocabularyEntries WHERE Word LIKE ? COLLATE NOCASE;");
};
