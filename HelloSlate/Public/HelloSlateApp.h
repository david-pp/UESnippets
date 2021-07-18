// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ISlateReflectorModule.h"
#include "HelloSlate.h"
#include "Widgets/Docking/SDockTab.h"


/**
 * Run the HelloSlate .
 */
int RunHelloSlate(const TCHAR* Commandline);

/**
 * Spawn the contents of the web browser tab
 */
TSharedRef<SDockTab> SpawnWebBrowserTab(const FSpawnTabArgs& Args);
