using UnrealBuildTool;

public class EnhanceVocabularyDevice: ModuleRules
{
    public EnhanceVocabularyDevice(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "EnhanceVocabularyCore",
            "Slate",
            "SlateCore",
            "DesktopPlatform"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "Core", 
            "CoreUObject", 
            "Engine", 
            "EnhanceVocabularyCore",
            "Slate",
            "SlateCore",
            "DesktopPlatform"
        });

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PrivateDependencyModuleNames.Add("Launch");

            AdditionalPropertiesForReceipt.Add(
                "AndroidPlugin",
                System.IO.Path.Combine(ModuleDirectory, "EVDevice_Android_UPL.xml")
            );
        }
    }
}
