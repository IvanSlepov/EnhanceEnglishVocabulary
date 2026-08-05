#include "EVResponseParser.h"

#include "EVJsonUtils.h"
#include "EVResponseTypes.h"
#include "EVWordInputValidator.h"
#include "Dom/JsonObject.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
constexpr const TCHAR* FreeDictionaryProviderName = TEXT("FreeDictionary");
constexpr const TCHAR* MyMemoryProviderName = TEXT("MyMemory");

void AddRelations(const TArray<FString>& Values, const TCHAR* RelationType, int32& DisplayOrder,
                  TSet<FString>& SeenRelations, TArray<FEVVocabularyRelation>& OutRelations)
{
    for (const FString& Value : Values)
    {
        const FString RelatedWord = Value.TrimStartAndEnd();
        const FString NormalizedRelatedWord = FEVWordInputValidator::NormalizeWordInput(RelatedWord);

        if (NormalizedRelatedWord.IsEmpty())
        {
            continue;
        }

        const FString DeduplicationKey = FString(RelationType) + TEXT("|") + NormalizedRelatedWord;
        if (SeenRelations.Contains(DeduplicationKey))
        {
            continue;
        }

        SeenRelations.Add(DeduplicationKey);

        FEVVocabularyRelation Relation;
        Relation.RelatedWord = RelatedWord;
        Relation.NormalizedRelatedWord = NormalizedRelatedWord;
        Relation.RelationType = RelationType;
        Relation.DisplayOrder = DisplayOrder++;
        Relation.ProviderName = FreeDictionaryProviderName;
        OutRelations.Add(MoveTemp(Relation));
    }
}
} // namespace

bool FEVResponseParser::ParseFreeDictionaryResponse(const FString& JsonString, FEVVocabularyRecord& OutRecord)
{
    OutRecord = FEVVocabularyRecord();

    TArray<TSharedPtr<FJsonValue>> JsonArray;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, JsonArray) || JsonArray.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse FreeDictionary JSON array."));
        return false;
    }

    TSet<FString> SeenPronunciations;
    TMap<FString, int32> MeaningIndexByPartOfSpeech;
    int32 PronunciationDisplayOrder = 0;
    int32 MeaningDisplayOrder = 0;

    for (const TSharedPtr<FJsonValue>& JsonValue : JsonArray)
    {
        if (!JsonValue.IsValid() || !JsonValue->AsObject().IsValid())
        {
            continue;
        }

        FEVFreeDictionaryResponse Response;
        if (!FJsonObjectConverter::JsonObjectToUStruct(JsonValue->AsObject().ToSharedRef(), &Response))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to convert FreeDictionary object to struct."));
            return false;
        }

        if (OutRecord.Word.IsEmpty())
        {
            OutRecord.Word = FEVWordInputValidator::NormalizeWordInput(Response.Word);
            OutRecord.NormalizedWord = OutRecord.Word;
        }

        auto AddPronunciation = [&](const FString& Transcription, const FString& AudioUrl, const FString& SourceUrl,
                                    const FEVFreeDictionaryLicense& License)
        {
            const FString CleanTranscription = Transcription.TrimStartAndEnd();
            const FString CleanAudioUrl = AudioUrl.TrimStartAndEnd();

            if (CleanTranscription.IsEmpty() && CleanAudioUrl.IsEmpty())
            {
                return;
            }

            const FString DeduplicationKey = CleanTranscription + TEXT("|") + CleanAudioUrl;
            if (SeenPronunciations.Contains(DeduplicationKey))
            {
                return;
            }

            SeenPronunciations.Add(DeduplicationKey);

            FEVVocabularyPronunciation Pronunciation;
            Pronunciation.LanguageCode = TEXT("en");
            Pronunciation.Transcription = CleanTranscription;
            Pronunciation.AudioUrl = CleanAudioUrl;
            Pronunciation.SourceUrl = SourceUrl.TrimStartAndEnd();
            Pronunciation.LicenseName = License.Name.TrimStartAndEnd();
            Pronunciation.LicenseUrl = License.Url.TrimStartAndEnd();
            Pronunciation.bPrimary = OutRecord.Pronunciations.IsEmpty();
            Pronunciation.DisplayOrder = PronunciationDisplayOrder++;
            Pronunciation.ProviderName = FreeDictionaryProviderName;
            OutRecord.Pronunciations.Add(MoveTemp(Pronunciation));
        };

        AddPronunciation(Response.Phonetic, FString(), FString(), Response.License);
        for (const FEVFreeDictionaryPhonetic& Phonetic : Response.Phonetics)
        {
            AddPronunciation(Phonetic.Text, Phonetic.Audio, Phonetic.SourceUrl, Phonetic.License);
        }

        for (const FEVFreeDictionaryMeaningGroup& ProviderMeaning : Response.Meanings)
        {
            FString PartOfSpeech = ProviderMeaning.PartOfSpeech.TrimStartAndEnd().ToLower();
            if (PartOfSpeech.IsEmpty())
            {
                PartOfSpeech = TEXT("unspecified");
            }

            int32 MeaningIndex = INDEX_NONE;
            if (const int32* ExistingMeaningIndex = MeaningIndexByPartOfSpeech.Find(PartOfSpeech))
            {
                MeaningIndex = *ExistingMeaningIndex;
            }
            else
            {
                FEVVocabularyMeaning Meaning;
                Meaning.PartOfSpeech = PartOfSpeech;
                Meaning.DisplayOrder = MeaningDisplayOrder++;
                Meaning.ProviderName = FreeDictionaryProviderName;
                MeaningIndex = OutRecord.Meanings.Add(MoveTemp(Meaning));
                MeaningIndexByPartOfSpeech.Add(PartOfSpeech, MeaningIndex);
            }

            FEVVocabularyMeaning& Meaning = OutRecord.Meanings[MeaningIndex];
            TSet<FString> SeenRelations;
            for (const FEVVocabularyRelation& ExistingRelation : Meaning.Relations)
            {
                SeenRelations.Add(ExistingRelation.RelationType + TEXT("|") + ExistingRelation.NormalizedRelatedWord);
            }
            int32 RelationDisplayOrder = Meaning.Relations.Num();

            AddRelations(ProviderMeaning.Synonyms, TEXT("synonym"), RelationDisplayOrder, SeenRelations,
                         Meaning.Relations);
            AddRelations(ProviderMeaning.Antonyms, TEXT("antonym"), RelationDisplayOrder, SeenRelations,
                         Meaning.Relations);

            for (const FEVFreeDictionaryDefinitionItem& ProviderDefinition : ProviderMeaning.Definitions)
            {
                const FString DefinitionText = ProviderDefinition.Definition.TrimStartAndEnd();
                const FString UsageExample = ProviderDefinition.Example.TrimStartAndEnd();

                if (!DefinitionText.IsEmpty() || !UsageExample.IsEmpty())
                {
                    FEVVocabularyDefinition Definition;
                    Definition.DefinitionText = DefinitionText;
                    Definition.UsageExample = UsageExample;
                    Definition.DisplayOrder = Meaning.Definitions.Num();
                    Definition.ProviderName = FreeDictionaryProviderName;
                    Meaning.Definitions.Add(MoveTemp(Definition));
                }

                AddRelations(ProviderDefinition.Synonyms, TEXT("synonym"), RelationDisplayOrder, SeenRelations,
                             Meaning.Relations);
                AddRelations(ProviderDefinition.Antonyms, TEXT("antonym"), RelationDisplayOrder, SeenRelations,
                             Meaning.Relations);
            }
        }
    }

    if (OutRecord.Word.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("FreeDictionary response did not contain a word."));
        return false;
    }

    return true;
}

bool FEVResponseParser::ParseMyMemoryTranslationResponse(const FString& JsonString, const FString& TargetLanguage,
                                                         FEVVocabularyTranslation& OutTranslation)
{
    OutTranslation = FEVVocabularyTranslation();

    FEVMyMemoryResponse Response;
    if (!FEVJsonUtils::JsonStringToStruct(JsonString, Response))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse MyMemory response."));
        return false;
    }

    if (Response.ResponseStatus != 200)
    {
        UE_LOG(LogTemp, Error, TEXT("MyMemory response status is not 200: %d"), Response.ResponseStatus);
        return false;
    }

    FString TranslationText;
    float Confidence = 0.0f;

    if (IsValidTranslationCandidate(Response.ResponseData.TranslatedText))
    {
        TranslationText = Response.ResponseData.TranslatedText.TrimStartAndEnd();
        Confidence = Response.ResponseData.Match;
    }
    else
    {
        for (const FEVMyMemoryMatchItem& Match : Response.Matches)
        {
            if (IsValidTranslationCandidate(Match.Translation))
            {
                TranslationText = Match.Translation.TrimStartAndEnd();
                Confidence = Match.Match;
                break;
            }
        }
    }

    if (TranslationText.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid MyMemory translation found."));
        return false;
    }

    OutTranslation.TranslationText = TranslationText;
    OutTranslation.TargetLanguage = TargetLanguage.ToLower();
    OutTranslation.ProviderName = MyMemoryProviderName;
    OutTranslation.Confidence = Confidence;
    return true;
}

bool FEVResponseParser::IsValidTranslationCandidate(const FString& Translation)
{
    const FString Clean = Translation.TrimStartAndEnd();

    if (Clean.IsEmpty() || Clean == TEXT("?") || Clean.Contains(TEXT("<g id=")))
    {
        return false;
    }

    return true;
}
