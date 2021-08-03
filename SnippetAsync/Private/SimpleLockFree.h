#pragma once

#include "Core.h"


class FMyData
{
public:
	FMyData(const int& V) : Value(V)
	{
	}

	int Value;
};


inline void Test_LockFree_LIFO()
{
	TLockFreePointerListLIFO<FMyData> ThreadSafeStack;

	TArray<TFuture<void>> Futures;
	const int ThreadNum = 10;

	// Create N threads
	for (int i = 0; i < ThreadNum; ++i)
	{
		TFuture<void> Future = Async(EAsyncExecution::Thread, [&ThreadSafeStack, i]()
		{
			// Push in a thread
			ThreadSafeStack.Push(new FMyData(i));

			// FPlatformProcess::Sleep(2);
			int32 ThreadId = FPlatformTLS::GetCurrentThreadId();
			FString ThreadName = FThreadManager::Get().GetThreadName(ThreadId);
			UE_LOG(LogTemp, Display, TEXT("%s[%d], Push, %d"), *ThreadName, ThreadId, i);
		});

		Futures.Add(MoveTemp(Future));
	}

	// Waiting for thread done
	while (true)
	{
		bool IsAllThreadDone = true;
		for (auto& Future : Futures)
		{
			if (!Future.IsReady())
				IsAllThreadDone = false;
		}

		if (IsAllThreadDone) break;
	}


	// Dump all in main thread
	while (FMyData* Data = ThreadSafeStack.Pop())
	{
		int32 ThreadId = FPlatformTLS::GetCurrentThreadId();
		FString ThreadName = FThreadManager::Get().GetThreadName(ThreadId);
		UE_LOG(LogTemp, Display, TEXT("%s[%d], Dump - %d"), *ThreadName, ThreadId, Data->Value);
		delete Data;
	}

	UE_LOG(LogTemp, Display, TEXT("Over ....."));
}

inline void Test_LockFree_FIFO()
{
	TLockFreePointerListFIFO<FMyData, PLATFORM_CACHE_LINE_SIZE> ThreadSafeList;

	TArray<TFuture<void>> Futures;
	const int ThreadNum = 10;

	// Create N threads
	for (int i = 0; i < ThreadNum; ++i)
	{
		TFuture<void> Future = Async(EAsyncExecution::Thread, [&ThreadSafeList, i]()
		{
			// Push in a thread
			ThreadSafeList.Push(new FMyData(i));

			// FPlatformProcess::Sleep(2);
			int32 ThreadId = FPlatformTLS::GetCurrentThreadId();
			FString ThreadName = FThreadManager::Get().GetThreadName(ThreadId);
			UE_LOG(LogTemp, Display, TEXT("%s[%d], Push, %d"), *ThreadName, ThreadId, i);
		});

		Futures.Add(MoveTemp(Future));
	}

	// Waiting for thread done
	while (true)
	{
		bool IsAllThreadDone = true;
		for (auto& Future : Futures)
		{
			if (!Future.IsReady())
				IsAllThreadDone = false;
		}

		if (IsAllThreadDone) break;
	}


	// Dump all in main thread
	while (FMyData* Data = ThreadSafeList.Pop())
	{
		int32 ThreadId = FPlatformTLS::GetCurrentThreadId();
		FString ThreadName = FThreadManager::Get().GetThreadName(ThreadId);
		UE_LOG(LogTemp, Display, TEXT("%s[%d], Dump - %d"), *ThreadName, ThreadId, Data->Value);
		delete Data;
	}

	UE_LOG(LogTemp, Display, TEXT("Over ....."));
}