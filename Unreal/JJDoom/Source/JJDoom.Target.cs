using UnrealBuildTool;
using System.Collections.Generic;

public class JJDoomTarget : TargetRules
{
	public JJDoomTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("JJDoom");
	}
}
