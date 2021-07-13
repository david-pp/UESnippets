// Copyright Epic Games, Inc. All Rights Reserved.


#include "SnippetAsync.h"

#include "RequiredProgramMainCPPInclude.h"

DEFINE_LOG_CATEGORY_STATIC(LogSnippetAsync, Log, All);

IMPLEMENT_APPLICATION(SnippetAsync, "SnippetAsync");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogSnippetAsync, Display, TEXT("Hello World! by David !!"));
	return 0;
}
