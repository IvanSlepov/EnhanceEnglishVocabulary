#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EVVocabularyLanguageTypes.h"
#include "EVVocabularyLanguageUiData.generated.h"

class UTexture2D;

/**
 * Shared presentation assets for language-aware widgets.
 * Create one DataAsset in Content, assign the flag textures once, then reference
 * the same asset from translation items and translation chips.
 */
UCLASS(BlueprintType)
class ENHANCEVOCABULARYUI_API UEVVocabularyLanguageUiData : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Language Flags")
    TMap<EEVVocabularyTranslationLanguage, TObjectPtr<UTexture2D>> LanguageFlags;

    UFUNCTION(BlueprintPure, Category = "Language Flags")
    UTexture2D* GetFlagTexture(EEVVocabularyTranslationLanguage Language) const;
};
