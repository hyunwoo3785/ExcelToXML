// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class OpenXLSX : ModuleRules
{
    public OpenXLSX(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            //var isDebug = (Target.Configuration == UnrealTargetConfiguration.Debug) || (Target.Configuration == UnrealTargetConfiguration.DebugGame);

            //if (isDebug)
            //{
            //  PublicAdditionalLibraries.Add("$(ModuleDir)/lib/debug/OpenXLSX.lib");
            //}
            //else
            {
                PublicAdditionalLibraries.Add("$(ModuleDir)/lib/OpenXLSX.lib");
                PublicAdditionalLibraries.Add("$(ModuleDir)/lib/tinyxml2.lib");

                PublicSystemIncludePaths.Add("$(ModuleDir)/include");
                PublicSystemIncludePaths.Add("$(ModuleDir)/");
            }
        }
    }
}
