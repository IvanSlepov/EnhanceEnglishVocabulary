#pragma once

#include "CoreMinimal.h"

class ENHANCEVOCABULARYSTORAGE_API FEVVocabularySqlQueries
{
public:
    static constexpr const TCHAR* CreateVocabularyTable = TEXT("CREATE TABLE IF NOT EXISTS VocabularyEntries ("
                                                               "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                                               "Word TEXT NOT NULL UNIQUE,"
                                                               "Definition TEXT,"
                                                               "Usage TEXT,"
                                                               "TranslationRu TEXT,"
                                                               "TranslationUa TEXT,"
                                                               "CreatedAt TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                                                               "UpdatedAt TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"
                                                               ");");

    static constexpr const TCHAR* InsertVocabularyEntry =
        TEXT("INSERT INTO VocabularyEntries "
             "(Word, Definition, Usage, TranslationRu, TranslationUa) "
             "VALUES (?, ?, ?, ?, ?) "
             "ON CONFLICT(Word) DO NOTHING;");

    static constexpr const TCHAR* SelectVocabularyEntries =
        TEXT("SELECT Word, Definition, Usage, TranslationRu, TranslationUa "
             "FROM VocabularyEntries "
             "ORDER BY Word ASC;");

    static constexpr const TCHAR* CountVocabularyEntries = TEXT("SELECT COUNT(*) FROM VocabularyEntries;");

    static constexpr const TCHAR* WordExists = TEXT("SELECT 1 FROM VocabularyEntries WHERE Word = ? LIMIT 1;");
};