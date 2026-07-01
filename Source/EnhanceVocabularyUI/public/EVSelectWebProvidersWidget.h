#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EVWebProviderTypes.h"
#include "EVSelectWebProvidersWidget.generated.h"

class UComboBoxString;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWebProvidersSelectionChanged, EEVWebProvider, DefinitionUsageProvider,
                                             EEVWebProvider, TranslationProvider);

UCLASS()
class ENHANCEVOCABULARYUI_API UEVSelectWebProvidersWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UComboBoxString> ComboBoxString_Definition_Usage_Providers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UComboBoxString> ComboBoxString_Translation_Providers;

    EEVWebProvider GetDefinitionUsageProvider() const;
    EEVWebProvider GetTranslationProvider() const;

    UPROPERTY(BlueprintAssignable)
    FOnWebProvidersSelectionChanged OnWebProvidersSelectionChanged;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

private:
    void PopulateProviderComboBoxes();

    UFUNCTION()
    void HandleDefinitionUsageProviderChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void HandleTranslationProviderChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    void BroadcastSelectedProviders();

    FString ProviderEnumToString(EEVWebProvider Provider) const;
    EEVWebProvider ProviderStringToEnum(const FString& ProviderName) const;
};