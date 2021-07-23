#pragma once

#include "Core.h"

class FPrimeNumberTask
{
public:
	// OUTPUT RESULTS OF TASKS
	static TArray<int32> PrimeNumbers;
	static FCriticalSection PrimeNumbersMutex;

	// This is the array of thread completions to know if all threads are done yet
	static FGraphEventArray CompletionEvents;

	static void FindPrimes(const uint32 TotalToFind)
	{
		PrimeNumbers.Empty();
		PrimeNumbers.Add(2);
		PrimeNumbers.Add(3);

		//~~~~~~~~~~~~~~~~~~~~
		//Add thread / task for each of total prime numbers to find
		//~~~~~~~~~~~~~~~~~~~~
		CompletionEvents.Empty();
		for (uint32 b = 0; b < TotalToFind; b++)
		{
			CompletionEvents.Add(
				TGraphTask<FPrimeNumberTask>::CreateTask(NULL, ENamedThreads::GameThread).
				ConstructAndDispatchWhenReady());
		}
	}

	static bool TasksAreComplete()
	{
		//Check all thread completion events
		for (int32 Index = 0; Index < CompletionEvents.Num(); Index++)
		{
			if (!CompletionEvents[Index]->IsComplete())
			{
				return false;
			}
		}
		return true;
	}

public:
	FPrimeNumberTask()
	{
	}

	~FPrimeNumberTask()
	{
	}

	FORCEINLINE static TStatId GetStatId()
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FPrimeNumberTask, STATGROUP_TaskGraphTasks);
	}

	static ENamedThreads::Type GetDesiredThread()
	{
		return ENamedThreads::AnyThread;
	}

	static ESubsequentsMode::Type GetSubsequentsMode()
	{
		return ESubsequentsMode::TrackSubsequents;
	}

	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
	{
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		PrimeNumber = FindNextPrimeNumber();

		{
			FScopeLock Lock(&PrimeNumbersMutex);
			PrimeNumbers.Add(PrimeNumber);
		}

		//  Please note you should not create, destroy, or modify UObjects here.
		//  Do those sort of things after all thread are completed.

		//  All calcs for making stuff can be done in the threads
		//  But the actual making/modifying of the UObjects should be done in main game thread,
		//  which is AFTER all tasks have completed :)
		Log("Find A Prime");
	}

protected:
	int32 FindNextPrimeNumber();

	void Log(const char* Action)
	{
		uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
		FString CurrentThreadName = FThreadManager::Get().GetThreadName(CurrentThreadId);
		UE_LOG(LogTemp, Display, TEXT("%s[%d] - %s, PrimeNumber:%d"), *CurrentThreadName, CurrentThreadId,
		       ANSI_TO_TCHAR(Action), PrimeNumber);
	}

	int32 PrimeNumber;
};


inline void Test_FindPrimeNumbers()
{
	FPrimeNumberTask::FindPrimes(5000); //first 50,000 prime numbers

	while (true)
	{
		if (FPrimeNumberTask::TasksAreComplete())
		{
			UE_LOG(LogTemp, Display, TEXT("Find Prime Number Task Are Complete !!"));

			for (int32 v = 0; v < FPrimeNumberTask::PrimeNumbers.Num(); v++)
			{
				UE_LOG(LogTemp, Display, TEXT("%d ~ %d"), v, FPrimeNumberTask::PrimeNumbers[v]);
			}

			break;
		}

		FPlatformProcess::Sleep(0.1);
	}

	UE_LOG(LogTemp, Display, TEXT("Over........"));
}
