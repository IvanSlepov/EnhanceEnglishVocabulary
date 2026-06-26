using UnrealBuildTool;

public class EnhanceVocabularyWeb: ModuleRules
{
    public EnhanceVocabularyWeb(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Core", 
            "CoreUObject", 
            "Engine",
            "EnhanceVocabularyCore",
            "HTTP",
            "Json",
            "JsonUtilities"
        });

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "EnhanceVocabularyCore",
            "HTTP",
            "Json",
            "JsonUtilities"
        });
    }
}
