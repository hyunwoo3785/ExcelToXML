// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ExcelToXMLTool : ModuleRules
{
	public ExcelToXMLTool(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        //PublicSystemIncludePaths.Add("$(ModuleDir)/Private/OpenXLSX/include");
        //PublicSystemIncludePaths.Add("$(ModuleDir)/Private/OpenXLSX/thirdparty/nowide");
        //PublicSystemIncludePaths.Add("$(ModuleDir)/Private/OpenXLSX/thirdparty/pugixml");
        //PublicSystemIncludePaths.Add("$(ModuleDir)/Private/OpenXLSX/thirdparty/zippy");

        PublicIncludePaths.AddRange(
			new string[] {

				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {

				// ... add other private include paths required here ...
			}
            );
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"EditorSubsystem"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"EditorFramework",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"OpenXLSX",
                "Blutility",
				"UMG",
                "UMGEditor"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
