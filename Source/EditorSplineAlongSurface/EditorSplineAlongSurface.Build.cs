using UnrealBuildTool;

public class EditorSplineAlongSurface : ModuleRules
{
	public EditorSplineAlongSurface(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
		PrivateDependencyModuleNames.AddRange(new string[] { "EditorScriptableToolsFramework", "SplineAlongSurface" });
	}
}