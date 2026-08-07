#pragma once

#include "CoreMinimal.h"
#include "EVVocabularyTypes.h"
#include "EVVocabularyInteractionTypes.generated.h"

UENUM(BlueprintType)
enum class EEVVocabularyValueItemMode : uint8
{
    ReviewReadOnly,
    DetailedReadOnly,
    DetailedEditable,
    FilterRemovableReadOnly
};

UENUM(BlueprintType)
enum class EEVVocabularyMeaningWidgetMode : uint8
{
    ReviewReadOnly,
    DetailedReadOnly,
    DetailedEditable
};

UENUM(BlueprintType)
enum class EEVVocabularyValueActionType : uint8
{
    Unknown,
    Translation,
    Synonym,
    Antonym
};

UENUM(BlueprintType)
enum class EEVVocabularyLanguageSupportState : uint8
{
    Unknown,
    Unsupported,
    SupportedWithoutContext,
    SupportedWithContext
};

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYCORE_API FEVVocabularyLanguageContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LanguageCode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEVVocabularyLanguageSupportState SupportState = EEVVocabularyLanguageSupportState::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DatabaseContextId;
};

USTRUCT(BlueprintType)
struct ENHANCEVOCABULARYCORE_API FEVVocabularyValueActionRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEVVocabularyValueActionType ActionType = EEVVocabularyValueActionType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SourceWord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FEVVocabularyTranslation Translation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FEVVocabularyRelation Relation;
};
