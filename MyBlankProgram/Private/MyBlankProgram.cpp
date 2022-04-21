// Copyright Epic Games, Inc. All Rights Reserved.


#include "MyBlankProgram.h"
#include "MyBlankObject.h"
#include "MyLog.h"
#include "RequiredProgramMainCPPInclude.h"
#include "TextFilterUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogMyBlankProgram, Log, All);

IMPLEMENT_APPLICATION(MyBlankProgram, "MyBlankProgram");

void ExampleString()
{
	FString String1 = TEXT("1-1-400 -80-0 |	2-1-800-20-0-0 | ");
	FString String2 = TEXT("1-1-400-80|2-1-800-20-0-0");


	TArray<FString> EffectsArray;
	String1.Replace(TEXT(" "), TEXT("")).Replace(TEXT("\t"), TEXT("")).ParseIntoArray(EffectsArray, TEXT("|"));
	for (auto& EffectStr : EffectsArray)
	{
		UE_LOG(LogMyBlankProgram, Log, TEXT("[%s]"), *EffectStr);

		TArray<FString> Elements;
		EffectStr.ParseIntoArray(Elements, TEXT("-"));

		int Type = 0;
		int DurationPolicy = 0;
		int DurationMs = 0;
		TArray<float> Coefficients;

		if (Elements.Num() > 0) Type = FCString::Atoi(*Elements[0]);
		if (Elements.Num() > 1) DurationPolicy = FCString::Atoi(*Elements[1]);
		if (Elements.Num() > 2) DurationMs = FCString::Atoi(*Elements[2]);

		UE_LOG(LogMyBlankProgram, Log, TEXT("effect:%d,%d,%d"), Type, DurationPolicy, DurationMs);
	}

	bool remove = true;
	// FString A = String1.TrimChar(TCHAR('|'), &remove);
	FString A = String1.Replace(TEXT(" "), TEXT(""));
	UE_LOG(LogMyBlankProgram, Log, TEXT("%s, %d"), *A, remove);


	FString Description = TEXT("物防+{X}%，持续{T}秒");
	FText Result = FText::FormatNamed(FText::FromString(Description), TEXT("X"),
	                                  FText::FromString(FString::FromInt(10)));
	UE_LOG(LogMyBlankProgram, Log, TEXT("%s"), *Result.ToString());

	//
	// FName
	// 
	FName UserName = TEXT("David");
	FString UserNameStr = UserName.ToString();
	FText UserNameText = FText::FromName(UserName);

	FName UserName2(FString(TEXT("David2")));
	FName UserName3(UserNameText.ToString());

	// case-insentive
	if (UserName == UserName3 && UserName.Compare(UserName3) == 0 && UserName == FName(TEXT("david")))
	{
		UE_LOG(LogMyBlankProgram, Log, TEXT("FName - UserName:%s == UserName3:%s"), *UserName.ToString(),
		       *UserName3.ToString());
	}

	FName EmptyName;
	if (FName(TEXT("david"), FNAME_Find) != NAME_None && EmptyName == NAME_None)
	{
		UE_LOG(LogMyBlankProgram, Log, TEXT("FName - Exist Name in name table : %d,%d"),
		       GetTypeHash(FName(TEXT("david"))),
		       GetTypeHash(UserName));
	}
}

void ExampleRegex()
{
	// Simple Example
	FString TextStr(TEXT("ABCDEFGHIJKLMN"));
	FRegexPattern TestPattern(TEXT("C.+H"));
	FRegexMatcher TestMatcher(TestPattern, TextStr);
	if (TestMatcher.FindNext())
	{
		int Begin = TestMatcher.GetMatchBeginning();
		int End = TestMatcher.GetMatchEnding();
		UE_LOG(LogMyBlankProgram, Log, TEXT("Find Matched : %d-%d -> %s"),
		       TestMatcher.GetMatchBeginning(), TestMatcher.GetMatchEnding(),
		       *TextStr.Mid(Begin, End - Begin));
	}

	// Complex Example
	FString String2 = TEXT("1-1-400-80|2-1-800-20-0-0");
}

void MainBody()
{
	UE_LOG(LogMyBlankProgram, Display, TEXT("Hello World! by David !!!"));

	ExampleLogging();

	ExampleString();

	ExampleRegex();

	UMyBlankObject::ExampleUObject();
	UMyBlankObject::ExampleReflection();
}

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	// Set your engine dir
	GForeignEngineDir = TEXT("/Users/david/UnrealEngine/Engine/");
	printf("Engine: > %s : %s : %s\n", UE_ENGINE_DIRECTORY, TCHAR_TO_ANSI(GForeignEngineDir),
	       TCHAR_TO_ANSI(*FPaths::EngineDir()));

	GEngineLoop.PreInit(ArgC, ArgV);
	MainBody();
	GEngineLoop.AppExit();
	return 0;
}
