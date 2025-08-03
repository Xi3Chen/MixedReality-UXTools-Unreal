// Copyright (c) 2020 Microsoft Corporation.
// Licensed under the MIT License.

using UnrealBuildTool;

public class XRSimulation : ModuleRules
{
	public XRSimulation(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"HeadMountedDisplay",
				"XRBase",
				"InputCore"
			}
		);

		PrivateDependencyModuleNames.AddRange(new string[]
			{
				"AugmentedReality",
				"RHI",
				"RenderCore",
				"Slate",
				"SlateCore",
				"RHI"
			}
		);

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}
	}
}
