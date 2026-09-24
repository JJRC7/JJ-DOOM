using UnrealBuildTool;
using System.Collections.Generic;

public class JJDoomEditorTarget : TargetRules
{
	public JJDoomEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("JJDoom");
	}
}
