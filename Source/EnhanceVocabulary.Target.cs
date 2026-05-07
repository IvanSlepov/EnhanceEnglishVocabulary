// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class EnhanceVocabularyTarget : TargetRules
{
	public EnhanceVocabularyTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("EnhanceVocabulary");
        ExtraModuleNames.Add("EnhanceVocabularyCore");
        ExtraModuleNames.Add("EnhanceVocabularyUI");
        ExtraModuleNames.Add("EnhanceVocabularyWeb");
        ExtraModuleNames.Add("EnhanceVocabularyStorage");
        ExtraModuleNames.Add("EnhanceVocabularyDevice");
    }
}
