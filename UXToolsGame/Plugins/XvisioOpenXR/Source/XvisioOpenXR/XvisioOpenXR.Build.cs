// Copyright 2021 Varjo Technologies Oy. All rights reserved.

using UnrealBuildTool;
using System;
using System.IO;


public class XvisioOpenXR : ModuleRules
{
	private string GetLibsPath(string Platform, string libName)
	{
		return Path.Combine("$(PluginDir)", "Source", "XvisioOpenXR", "ThirdParty", Platform, libName);

	}
	public XvisioOpenXR(ReadOnlyTargetRules Target) : base(Target)
	{
		System.Console.WriteLine("xvisio-log...... start build.cs");
	
		// StrictIncludes mode from BuildPlugin automation
		PCHUsage = PCHUsageMode.NoPCHs;
		// bUseUnity = false;

		// bUseRTTI = true;
		// bEnableExceptions = true;
		// CppStandard = CppStandardVersion.Cpp17;
		
		// PublicDefinitions.Add("XR_USE_TIMESPEC=1");

		
		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine("XvisioOpenXR", "Private"),
			}
			);
			
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"DeveloperSettings",
				"InputDevice",
				// "LiveLink",
				// "LiveLinkInterface"
				// "AugmentedReality"
			}
			);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"Slate",
				"HeadMountedDisplay",
				"SlateCore",
			/* 	"LiveLink",
				"LiveLinkInterface", */
				// "AugmentedReality",
				"OpenXRHMD",
				 "OpenXRAR",
				"OpenXRInput",
				"XRBase",
				"Json"
				// "ProceduralMeshComponent",
			}
		);
		  AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenXR");
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PublicDefinitions.Add("XR_USE_PLATFORM_ANDROID=1");

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{ 
					"ApplicationCore", 
					"Launch",
				}
			);
			// AndroidPlugin
			{
				string ModulePath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
				AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModulePath, "XvisioOpenXR_UPL.xml"));
			}
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			string XvisioOpenXRSoPath = GetLibsPath("Linux", "libxv_client.so");
			PublicAdditionalLibraries.Add(XvisioOpenXRSoPath);
			PublicDelayLoadDLLs.Add(XvisioOpenXRSoPath);
			RuntimeDependencies.Add("$(TargetOutputDir)/libxv_client.so", XvisioOpenXRSoPath);
		}
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string XvisioOpenXRLibPath = GetLibsPath("Win64", "xv_client.lib");
			string XvisioOpenXRDllPath = GetLibsPath("Win64", "xv_client.dll");
			PublicAdditionalLibraries.Add(XvisioOpenXRLibPath);
			RuntimeDependencies.Add("$(TargetOutputDir)/xv_client.dll", XvisioOpenXRDllPath);
			PublicDelayLoadDLLs.Add(XvisioOpenXRDllPath);

			bEnableExceptions = true;
			bUseUnity = false;
			CppStandard = CppStandardVersion.Cpp17;
			PublicSystemLibraries.AddRange(new string[] { "shlwapi.lib", "runtimeobject.lib" });
			PrivateIncludePaths.Add(Path.Combine(Target.WindowsPlatform.WindowsSdkDir,
												"Include",
												Target.WindowsPlatform.WindowsSdkVersion,
												"cppwinrt"));

		}
	}
}

