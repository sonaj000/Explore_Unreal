// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TestGround : ModuleRules
{
	public TestGround(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput","OnlineSubsystem","OnlineSubsystemUtils", "UMG", "Json", "JsonUtilities", "NavigationSystem" , "UnrealEd", "AssetTools"});

        PrivateDependencyModuleNames.AddRange(new string[] { "SQLiteCore", "SQLiteSupport" });
    }
}
