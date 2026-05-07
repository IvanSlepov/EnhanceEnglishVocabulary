using UnrealBuildTool;

public class EnhanceVocabularyDevice: ModuleRules
{
    public EnhanceVocabularyDevice(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
