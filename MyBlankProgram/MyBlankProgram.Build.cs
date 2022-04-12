// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MyBlankProgram : ModuleRules
{
	public MyBlankProgram(ReadOnlyTargetRules Target) : base(Target)
	{
		
        PrivateIncludePaths.Add(System.IO.Path.Combine(EngineDirectory, "Source/Runtime/Launch/Public"));
		PrivateIncludePaths.Add(System.IO.Path.Combine(EngineDirectory, "Source/Runtime/Launch/Private"));		// For LaunchEngineLoop.cpp include

		PrivateDependencyModuleNames.Add("Core");
		PrivateDependencyModuleNames.Add("CoreUObject");
		PrivateDependencyModuleNames.Add("Projects");
	}
}
