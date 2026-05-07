using UnrealBuildTool;

public class EnhanceVocabularyCore: ModuleRules
{
    public EnhanceVocabularyCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Core", 
            "CoreUObject", 
            "Engine"
        });
    }
}
