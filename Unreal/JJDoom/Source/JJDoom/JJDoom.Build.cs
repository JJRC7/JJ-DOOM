using UnrealBuildTool;

public class JJDoom : ModuleRules
{
	public JJDoom(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AIModule" });

		// Las versiones nuevas de Visual Studio avisan de macros no definidas (C4668) dentro de las
		// cabeceras del motor; no son errores de este proyecto, así que se silencian.
		UndefinedIdentifierWarningLevel = WarningLevel.Off;
		bWarningsAsErrors = false;
	}
}
