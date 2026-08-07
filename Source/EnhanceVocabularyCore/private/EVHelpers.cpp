// Fill out your copyright notice in the Description page of Project Settings.

#include "EVHelpers.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "JsonObjectConverter.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/Guid.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Misc/Base64.h"

FString UEVHelpers::GetVocabularyDebugDbPath()
{
    return FPaths::Combine(FPaths::ProjectContentDir(), TEXT("VocabularyDB"), TEXT("DebugDBFile"),
                           TEXT("EnhanceVocabulary_Debug_Template.db"));
}

FString UEVHelpers::GetVocabularyCleanDbPath()
{
    return FPaths::Combine(FPaths::ProjectContentDir(), TEXT("VocabularyDB"), TEXT("CleanDBFile"),
                           TEXT("EnhanceVocabulary_Clean_Template.db"));
}

FString UEVHelpers::GetVocabularyLiveDbPath()
{
    return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("EnhanceVocabulary.db"));
}

FString UEVHelpers::GetVocabularyDebugDbPathForContext(const EEVVocabularyDBContext Context)
{
    // English USA intentionally keeps the existing debug DB path for full backward compatibility.
    if (Context == EEVVocabularyDBContext::EnglishUSA)
    {
        return GetVocabularyDebugDbPath();
    }

    return FPaths::Combine(
        FPaths::ProjectContentDir(), TEXT("VocabularyDB"), TEXT("DebugDBFile"),
        FString::Printf(TEXT("EnhanceVocabulary_%s_Debug.db"), *EVVocabularyLanguage::GetDatabaseContextId(Context)));
}

FString UEVHelpers::GetVocabularyLiveDbPathForContext(const EEVVocabularyDBContext Context)
{
    // English USA intentionally keeps the existing live DB filename so current users do not lose their vocabulary.
    if (Context == EEVVocabularyDBContext::EnglishUSA)
    {
        return GetVocabularyLiveDbPath();
    }

    return FPaths::Combine(
        FPaths::ProjectSavedDir(),
        FString::Printf(TEXT("EnhanceVocabulary_%s.db"), *EVVocabularyLanguage::GetDatabaseContextId(Context)));
}
