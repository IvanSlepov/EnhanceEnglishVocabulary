#include "EVSelectWebProvidersWidget.h"

#include "Components/ComboBoxString.h"
#include "EVWebProviderRegistry.h"
#include "UObject/UnrealType.h"

void UEVSelectWebProvidersWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
}

void UEVSelectWebProvidersWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UEVSelectWebProvidersWidget::NativeConstruct()
{
    Super::NativeConstruct();

    PopulateProviderComboBoxes();

    if (ComboBoxString_Definition_Usage_Providers)
    {
        ComboBoxString_Definition_Usage_Providers->OnSelectionChanged.AddDynamic(
            this, &ThisClass::HandleDefinitionUsageProviderChanged);
    }

    if (ComboBoxString_Translation_Providers)
    {
        ComboBoxString_Translation_Providers->OnSelectionChanged.AddDynamic(
            this, &ThisClass::HandleTranslationProviderChanged);
    }

    BroadcastSelectedProviders();
}

void UEVSelectWebProvidersWidget::PopulateProviderComboBoxes()
{
    if (!ComboBoxString_Definition_Usage_Providers || !ComboBoxString_Translation_Providers)
    {
        UE_LOG(LogTemp, Error, TEXT("Provider combo boxes are not bound in EVSelectWebProvidersWidget"));
        return;
    }

    ComboBoxString_Definition_Usage_Providers->ClearOptions();
    ComboBoxString_Translation_Providers->ClearOptions();

    const TArray<FEVWebProviderInfo> AvailableProviders = FEVWebProviderRegistry::GetAvailableProviders();

    for (const FEVWebProviderInfo& ProviderInfo : AvailableProviders)
    {
        const FString ProviderName = ProviderEnumToString(ProviderInfo.Provider);

        switch (ProviderInfo.ProviderType)
        {
        case EEVWebProviderType::Dictionary:
            ComboBoxString_Definition_Usage_Providers->AddOption(ProviderName);
            break;

        case EEVWebProviderType::Translation:
            ComboBoxString_Translation_Providers->AddOption(ProviderName);
            break;

        default:
            break;
        }
    }

    ComboBoxString_Definition_Usage_Providers->SetSelectedOption(ProviderEnumToString(EEVWebProvider::FreeDictionary));

    ComboBoxString_Translation_Providers->SetSelectedOption(ProviderEnumToString(EEVWebProvider::MyMemory));
}

EEVWebProvider UEVSelectWebProvidersWidget::GetDefinitionUsageProvider() const
{
    if (!ComboBoxString_Definition_Usage_Providers)
    {
        return EEVWebProvider::FreeDictionary;
    }

    return ProviderStringToEnum(ComboBoxString_Definition_Usage_Providers->GetSelectedOption());
}

EEVWebProvider UEVSelectWebProvidersWidget::GetTranslationProvider() const
{
    if (!ComboBoxString_Translation_Providers)
    {
        return EEVWebProvider::MyMemory;
    }

    return ProviderStringToEnum(ComboBoxString_Translation_Providers->GetSelectedOption());
}

FString UEVSelectWebProvidersWidget::ProviderEnumToString(EEVWebProvider Provider) const
{
    const UEnum* EnumPtr = StaticEnum<EEVWebProvider>();

    if (!EnumPtr)
    {
        return TEXT("Unknown");
    }

    return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Provider)).ToString();
}

EEVWebProvider UEVSelectWebProvidersWidget::ProviderStringToEnum(const FString& ProviderName) const
{
    const UEnum* EnumPtr = StaticEnum<EEVWebProvider>();

    if (!EnumPtr)
    {
        return EEVWebProvider::FreeDictionary;
    }

    for (const FEVWebProviderInfo& Info : FEVWebProviderRegistry::GetAvailableProviders())
    {
        const FString DisplayName = EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Info.Provider)).ToString();

        if (DisplayName == ProviderName)
        {
            return Info.Provider;
        }
    }

    return EEVWebProvider::FreeDictionary;
}

void UEVSelectWebProvidersWidget::BroadcastSelectedProviders()
{
    OnWebProvidersSelectionChanged.Broadcast(GetDefinitionUsageProvider(), GetTranslationProvider());
}

void UEVSelectWebProvidersWidget::HandleDefinitionUsageProviderChanged(FString SelectedItem,
                                                                       ESelectInfo::Type SelectionType)
{
    BroadcastSelectedProviders();
}

void UEVSelectWebProvidersWidget::HandleTranslationProviderChanged(FString SelectedItem,
                                                                   ESelectInfo::Type SelectionType)
{
    BroadcastSelectedProviders();
}