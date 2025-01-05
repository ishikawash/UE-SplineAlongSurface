// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SplineAlongSurface : ModuleRules
{
	public SplineAlongSurface(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
		PrivateDependencyModuleNames.AddRange(new string[] { "GeometryScriptingCore" });
	}
}
