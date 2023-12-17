// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Scancraft : ModuleRules
{
	public Scancraft(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
       // PrivatePCHHeaderFile = "Private/WindowsMixedRealityPrecompiled.h";

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AdvancedSessions", "VRExpansionPlugin", "EnhancedInput", "Open3DUE5", "Voxel", "Open3D" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });
		bUseRTTI = true;

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
