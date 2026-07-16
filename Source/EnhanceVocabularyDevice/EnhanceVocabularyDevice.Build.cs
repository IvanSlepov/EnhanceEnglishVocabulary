using UnrealBuildTool;

public class EnhanceVocabularyDevice : ModuleRules
{
    public EnhanceVocabularyDevice(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "EnhanceVocabularyCore",
            "Slate",
            "SlateCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "EnhanceVocabularyCore",
            "Slate",
            "SlateCore"
        });

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PrivateDependencyModuleNames.Add("DesktopPlatform");
        }

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PrivateDependencyModuleNames.Add("Launch");

            AdditionalPropertiesForReceipt.Add(
                "AndroidPlugin",
                System.IO.Path.Combine(
                    ModuleDirectory,
                    "EVDevice_Android_UPL.xml")
            );
        }
    }
}