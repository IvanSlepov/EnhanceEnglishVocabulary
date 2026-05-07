// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class EnhanceVocabularyEditorTarget : TargetRules
{
	public EnhanceVocabularyEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
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
