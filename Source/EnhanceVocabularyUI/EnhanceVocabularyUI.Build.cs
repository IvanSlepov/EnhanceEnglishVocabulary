using UnrealBuildTool;

public class EnhanceVocabularyUI: ModuleRules
{
    public EnhanceVocabularyUI(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "UMG",
            "EnhanceVocabularyStorage",
            "EnhanceVocabulary",
            "EnhanceVocabularyCore",
            "EnhanceVocabularyStorage",
            "EnhanceVocabularyWeb"
        });

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "UMG",
            "Slate",
            "SlateCore",
            "EnhanceVocabularyStorage",
            "EnhanceVocabulary",
            "EnhanceVocabularyCore",
            "EnhanceVocabularyStorage",
            "EnhanceVocabularyWeb"
        });
    }
}
