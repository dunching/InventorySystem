// Copyright 2019 Tefel. All Rights Reserved

using System.IO;
using UnrealBuildTool;

public class Command_InventorySystem : ModuleRules
{
	public Command_InventorySystem(ReadOnlyTargetRules Target) : base(Target)
    {
	    // Include What You Use (IWYU)
	    bUseUnity = false;
	    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        DefaultBuildSettings = BuildSettingsVersion.Latest;

        CppStandard = CppStandardVersion.Cpp20;

        bUseRTTI = false;

        PublicIncludePaths.AddRange(
			new string[] {
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
            }
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"NetCore", 
				
				"ItemSystem",
				"Utils",
				"InventorySystem",
            }
			);

        DynamicallyLoadedModuleNames.AddRange(
			new string[]
            {
            }
			);
	}
}
