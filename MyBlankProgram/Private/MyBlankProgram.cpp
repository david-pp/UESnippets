// Copyright Epic Games, Inc. All Rights Reserved.


#include "MyBlankProgram.h"
#include "MyLog.h"
#include "RequiredProgramMainCPPInclude.h"

DEFINE_LOG_CATEGORY_STATIC(LogMyBlankProgram, Log, All);

IMPLEMENT_APPLICATION(MyBlankProgram, "MyBlankProgram");

void MainBody()
{
	UE_LOG(LogMyBlankProgram, Display, TEXT("Hello World! by David !!!"));

	ExampleLogging();
}

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	// Set your engine dir
	// GForeignEngineDir = TEXT("/Users/david/UnrealEngine/Engine/");
	printf("Engine: > %s : %s : %s\n", UE_ENGINE_DIRECTORY, TCHAR_TO_ANSI(GForeignEngineDir),
		   TCHAR_TO_ANSI(*FPaths::EngineDir()));
	
	GEngineLoop.PreInit(ArgC, ArgV);
	MainBody();
	GEngineLoop.AppExit();
	return 0;
}


