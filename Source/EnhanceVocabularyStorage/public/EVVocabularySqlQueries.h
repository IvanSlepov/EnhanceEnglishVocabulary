#pragma once

#include "CoreMinimal.h"

class ENHANCEVOCABULARYSTORAGE_API FEVVocabularySqlQueries
{
public:
    // We moved the DB creation from query to the dedicated function
    // in order to control the DB fields.
    // Later, we'll update rest of the queries too.
    static FString GetCreateVocabularyTableQuery();

    static constexpr const TCHAR* InsertVocabularyEntryStrict =
        TEXT("INSERT INTO VocabularyEntries "
             "(Word, Definition, Usage, TranslationRu, TranslationUa) "
             "VALUES (?, ?, ?, ?, ?);");

    static constexpr const TCHAR* EditVocabularyEntry = TEXT("UPDATE VocabularyEntries "
                                                             "SET Definition = ?, "
                                                             "    Usage = ?, "
                                                             "    TranslationRu = ?, "
                                                             "    TranslationUa = ? "
                                                             "WHERE Word = ?");

    static constexpr const TCHAR* DeleteVocabularyEntry = TEXT("DELETE FROM VocabularyEntries "
                                                               "WHERE Word = ?;");

    static constexpr const TCHAR* GetVocabularyEntryByWord =
        TEXT("SELECT Word, Definition, Usage, TranslationRu, TranslationUa "
             "FROM VocabularyEntries "
             "WHERE Word = ?;");

    // Use this func to make Importing/Exporting the DB less hardcoded column names
    // dependent
    static FString GetSelectImportExportColumnsQuery();

    static constexpr const TCHAR* SelectVocabularyEntries =
        TEXT("SELECT Word, Definition, Usage, TranslationRu, TranslationUa "
             "FROM VocabularyEntries "
             "ORDER BY Word ASC;");

    static constexpr const TCHAR* SelectVocabularyEntriesPage =
        TEXT("SELECT Word, Definition, Usage, TranslationRu, TranslationUa "
             "FROM VocabularyEntries "
             "ORDER BY Word COLLATE NOCASE ASC "
             "LIMIT ? OFFSET ?;");

    static constexpr const TCHAR* CountVocabularyEntries = TEXT("SELECT COUNT(*) FROM VocabularyEntries;");

    static constexpr const TCHAR* WordExists = TEXT("SELECT 1 FROM VocabularyEntries WHERE Word = ? LIMIT 1;");
};