using UnrealBuildTool;

public class EnhanceVocabularyStorage: ModuleRules
{
    public EnhanceVocabularyStorage(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
