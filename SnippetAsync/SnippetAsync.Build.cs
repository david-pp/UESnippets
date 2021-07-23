// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SnippetAsync : ModuleRules
{
	public SnippetAsync(ReadOnlyTargetRules Target) : base(Target)
	{
		
        PrivateIncludePaths.Add(System.IO.Path.Combine(EngineDirectory, "Source/Runtime/Launch/Public"));
		PrivateIncludePaths.Add(System.IO.Path.Combine(EngineDirectory, "Source/Runtime/Launch/Private"));		// For LaunchEngineLoop.cpp include

		PrivateDependencyModuleNames.Add("Core");
		PrivateDependencyModuleNames.Add("Projects");
	}
}
