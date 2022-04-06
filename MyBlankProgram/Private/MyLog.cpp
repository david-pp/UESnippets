#include "MyLog.h"

DEFINE_LOG_CATEGORY(LogMy);

void ExampleLogging()
{
	// UE_LOG(LogMy, Fatal, TEXT("致命错误")); // 会直接Crash掉
	UE_LOG(LogMy, Error, TEXT("错误"));
	UE_LOG(LogMy, Warning, TEXT("警告"));
	UE_LOG(LogMy, Display, TEXT("显示"));
	UE_LOG(LogMy, Log, TEXT("Log .."));
	UE_LOG(LogMy, Verbose, TEXT("Verbose Log .."));
	UE_LOG(LogMy, VeryVerbose, TEXT("Very Verbose Log ..."));
}
