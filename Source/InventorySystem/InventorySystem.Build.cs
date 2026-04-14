// Copyright 2019 Tefel. All Rights Reserved

using System.IO;
using UnrealBuildTool;

public class InventorySystem : ModuleRules
{
	public InventorySystem(ReadOnlyTargetRules Target) : base(Target)
    {
	    // Include What You Use (IWYU)
	    bUseUnity = false;
	    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        DefaultBuildSettings = BuildSettingsVersion.Latest;

        CppStandard = CppStandardVersion.Cpp20;

        bUseRTTI = false;

        PublicIncludePaths.AddRange(
			new string[] {
                   Path.Combine(ModuleDirectory, "InventorySystem"),
            }
			);
				
		PrivateIncludePaths.AddRange(
			new string[] {
            }
			);

        if (Target.bBuildEditor == true)
        {
	        PrivateDependencyModuleNames.Add("UnrealEd");
        }

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"Json",
				
				"ItemProxy",
            }
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"NetCore", 
				
				"ItemSystem",
				"Utils",
            }
			);

        DynamicallyLoadedModuleNames.AddRange(
			new string[]
            {
            }
			);
	}
}
