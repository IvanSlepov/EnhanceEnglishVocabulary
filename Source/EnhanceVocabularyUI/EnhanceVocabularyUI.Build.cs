using UnrealBuildTool;

public class EnhanceVocabularyUI: ModuleRules
{
    public EnhanceVocabularyUI(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
