using UnrealBuildTool;

public class EnhanceVocabularyCore: ModuleRules
{
    public EnhanceVocabularyCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Core", 
            "CoreUObject", 
            "Engine",
            "InputCore",
            "Json",
            "JsonUtilities",
            "JsonBlueprintUtilities"
        });

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "Json",
            "JsonUtilities",
            "JsonBlueprintUtilities",
        });
    }
}
