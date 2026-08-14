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
            "EnhanceVocabularyCore",
            "EnhanceVocabularyStorage",
            "EnhanceVocabularyWeb"
        });
    }
}
