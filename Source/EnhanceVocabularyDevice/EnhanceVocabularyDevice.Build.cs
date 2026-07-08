using UnrealBuildTool;

public class EnhanceVocabularyDevice: ModuleRules
{
    public EnhanceVocabularyDevice(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "EnhanceVocabularyCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "Core", 
            "CoreUObject", 
            "Engine", 
            "EnhanceVocabularyCore"
        });
    }
}
