using UnrealBuildTool;

public class UXToolsHandPoseEditor : ModuleRules
{
    public UXToolsHandPoseEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "HeadMountedDisplay",
                "UXToolsHandPose",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "ApplicationCore",
                "CoreUObject",
                "DeveloperSettings",
                "Engine",
                "InputCore",
                "LevelEditor",
                "PropertyEditor",
                "Slate",
                "SlateCore",
                "ToolMenus",
                "UnrealEd"
            }
        );
    }
}
