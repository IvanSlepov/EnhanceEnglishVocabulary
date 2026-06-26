#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "Dom/JsonObject.h"

class ENHANCEVOCABULARYCORE_API FEVJsonUtils
{

public:
    static bool JsonStringToObject(const FString& JsonString, TSharedPtr<FJsonObject>& OutJsonObject);

    static bool JsonObjectToString(const TSharedPtr<FJsonObject>& JsonObject, FString& OutJsonString);

    template <typename StructType> static bool JsonStringToStruct(const FString& JsonString, StructType& OutStruct);

    template <typename StructType> static bool StructToJsonString(const StructType& Struct, FString& OutJsonString);
};

template <typename StructType> bool FEVJsonUtils::JsonStringToStruct(const FString& JsonString, StructType& OutStruct)
{
    TSharedPtr<FJsonObject> JsonObject;

    if (!JsonStringToObject(JsonString, JsonObject))
    {
        return false;
    }

    return FJsonObjectConverter::JsonObjectToUStruct<StructType>(JsonObject.ToSharedRef(), &OutStruct);
}

template <typename StructType> bool FEVJsonUtils::StructToJsonString(const StructType& Struct, FString& OutJsonString)
{
    TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Struct);

    if (!JsonObject.IsValid())
    {
        return false;
    }

    return JsonObjectToString(JsonObject, OutJsonString);
}