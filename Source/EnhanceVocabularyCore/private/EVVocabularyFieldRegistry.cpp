#include "EVVocabularyFieldRegistry.h"

const TArray<FEVDatabaseColumnDefinition>& FEVVocabularyFieldRegistry::GetPersistentFields()
{
    // This is the single ordered mapping between FVocabularyEntry and SQLite/CSV.
    // Keep descriptors in the desired SELECT/INSERT/import/export order.
    static const TArray<FEVDatabaseColumnDefinition> Fields = {
        {TEXT("Word"), TEXT("TEXT NOT NULL UNIQUE"), EEVDatabaseColumnRole::ImportExport,
         EEVImportRequirement::Required, TEXT("WORD - can't be empty!!!"), &FVocabularyEntry::Word, false},
        {TEXT("Transcription"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("Transcription - can be empty"), &FVocabularyEntry::Transcription, true},
        {TEXT("Definition"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("Definition - can be empty"), &FVocabularyEntry::Definition, true},
        {TEXT("Usage"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("Usage - can be empty"), &FVocabularyEntry::Usage, true},
        {TEXT("TranslationRu"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("TranslationRU - can be empty"), &FVocabularyEntry::TranslationRu, true},
        {TEXT("TranslationUa"), TEXT("TEXT"), EEVDatabaseColumnRole::ImportExport, EEVImportRequirement::Optional,
         TEXT("TranslationUA - can be empty"), &FVocabularyEntry::TranslationUa, true},
    };

    return Fields;
}

TArray<FEVDatabaseColumnDefinition> FEVVocabularyFieldRegistry::GetEditablePersistentFields()
{
    TArray<FEVDatabaseColumnDefinition> Result;

    for (const FEVDatabaseColumnDefinition& Field : GetPersistentFields())
    {
        if (Field.bEditable)
        {
            Result.Add(Field);
        }
    }

    return Result;
}

TArray<FString> FEVVocabularyFieldRegistry::GetPersistentColumnNames()
{
    TArray<FString> Names;
    Names.Reserve(GetPersistentFields().Num());

    for (const FEVDatabaseColumnDefinition& Field : GetPersistentFields())
    {
        Names.Add(Field.Name);
    }

    return Names;
}
