// Copyright Epic Games, Inc. All Rights Reserved.


#include "MyBlankProgram.h"

#include "RequiredProgramMainCPPInclude.h"

DEFINE_LOG_CATEGORY_STATIC(LogMyBlankProgram, Log, All);

IMPLEMENT_APPLICATION(MyBlankProgram, "MyBlankProgram");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogMyBlankProgram, Display, TEXT("Hello World! by David !!"));
	return 0;
}
