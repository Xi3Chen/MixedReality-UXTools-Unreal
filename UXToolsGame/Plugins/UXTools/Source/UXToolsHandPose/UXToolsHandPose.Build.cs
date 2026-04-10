using UnrealBuildTool;

public class UXToolsHandPose : ModuleRules
{
    public UXToolsHandPose(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "ApplicationCore",
                "HeadMountedDisplay",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "InputDevice",
                "Slate",
                "SlateCore",
                "DeveloperSettings"
            }
        );
    }
}
