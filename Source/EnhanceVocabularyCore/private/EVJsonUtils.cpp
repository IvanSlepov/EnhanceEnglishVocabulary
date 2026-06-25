#include "EVJsonUtils.h"

#include "Dom/JsonObject.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

bool FEVJsonUtils::JsonStringToObject(const FString& JsonString, TSharedPtr<FJsonObject>& OutJsonObject)
{
    if (JsonString.IsEmpty())
    {
        return false;
    }

    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    return FJsonSerializer::Deserialize(Reader, OutJsonObject) && OutJsonObject.IsValid();
}

bool FEVJsonUtils::JsonObjectToString(const TSharedPtr<FJsonObject>& JsonObject, FString& OutJsonString)
{
    if (!JsonObject.IsValid())
    {
        return false;
    }

    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJsonString);

    return FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
}
