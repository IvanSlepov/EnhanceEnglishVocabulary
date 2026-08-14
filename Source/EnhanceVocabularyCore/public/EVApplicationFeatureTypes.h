#pragma once

#include "CoreMinimal.h"

namespace EVApplicationFeature
{
inline const FName None(TEXT("None"));
inline const FName MainMenu(TEXT("Navigation.MainMenu"));
inline const FName AddWord(TEXT("Vocabulary.AddWord"));
inline const FName ReviewWords(TEXT("Vocabulary.ReviewWords"));
inline const FName NotificationSettings(TEXT("Notifications.Settings"));
inline const FName ImportExport(TEXT("Vocabulary.ImportExport"));
inline const FName ApplicationSettings(TEXT("Application.Settings"));
inline const FName EntryDetails(TEXT("Vocabulary.EntryDetails"));
inline const FName VocabularyFilters(TEXT("Vocabulary.Filters"));
} // namespace EVApplicationFeature

struct ENHANCEVOCABULARYCORE_API FEVFeatureNavigationRequest
{
    FName FeatureId = EVApplicationFeature::None;
    FString Context;
};
