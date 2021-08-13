// Copyright Epic Games, Inc. All Rights Reserved.


#include "SnippetAsync.h"

#include "RequiredProgramMainCPPInclude.h"
#include "Async/Async.h"
#include "Async/AsyncWork.h"
#include <string>
#include <iostream>

#include "ParallelFor.h"

#include "SimpleThread.h"
#include "SimpleQueuedWorker.h"
#include "SimpleAsyncTask.h"
#include "SimpleGraphTask.h"
#include "SimpleAsync.h"

#include "RNGThread.h"
#include "PrimeNumberThread.h"
#include "PrimeNumberGraphTask.h"

#include "SimpleLockFree.h"
#include "SimpleProducerConsumer.h"


DEFINE_LOG_CATEGORY_STATIC(LogSnippetAsync, Log, All);

IMPLEMENT_APPLICATION(SnippetAsync, "SnippetAsync");


////////////////////////////////////////////////////

void TestParallel()
{
	int32 MaxEntries = 500;
	ParallelFor(MaxEntries, [](int32 CurrIdx)
	{
		// Your implementation may fetch/store results based on the CurrIdx, but
		// for simplicity we just have some dummy "expensive" calculation here.
		int32 Sum = CurrIdx;
		for (int32 Idx = 0; Idx < 1000 * 10; ++Idx)
		{
			Sum += FMath::Sqrt(1234.56f);
		}

		uint32 ThreadId = FPlatformTLS::GetCurrentThreadId();
		const FString& ThreadName = FThreadManager::GetThreadName(ThreadId);

		UE_LOG(LogTemp, Display, TEXT("sum ~ %d, thread:%d,%s"), Sum,
		       FPlatformTLS::GetCurrentThreadId(), *ThreadName);
	});
}

//////////////////////////////////////////////////

void TestRNGThread()
{
	// Create RNG Thread
	FRNGThread* RNG = new FRNGThread(50, 0, 100, 5);

	// Consume Random Vector from RNG Thread
	int TickCount = 100;
	for (int i = 0; i < TickCount; ++i)
	{
		// Consume
		FVector V = RNG->GetRandomVector();

		bool IsPaused = RNG->IsThreadPaused();
		UE_LOG(LogSnippetAsync, Display, TEXT("Random Vector[%d] : %s, FRNGThread Paused : %d"), i, *V.ToString(),
		       IsPaused);
		FPlatformProcess::Sleep(0.1);
	}

	// Destroy RNG Thread
	if (RNG)
	{
		RNG->EnsureCompletion();
		delete RNG;
		RNG = nullptr;
	}

	UE_LOG(LogSnippetAsync, Display, TEXT("RNG Thread is Over !!"));
}

void TestPrimeNumberThreads()
{
	TArray<int32> PrimeNumbers;
	FPrimeNumberThread::JoyInit(PrimeNumbers, 5000);

	while (!FPrimeNumberThread::IsThreadFinished())
	{
		UE_LOG(LogSnippetAsync, Display, TEXT("FPrimeNumberThread Thread is Working..."));
		FPlatformProcess::Sleep(0.1);
	}

	FPrimeNumberThread::Shutdown();
	UE_LOG(LogSnippetAsync, Display, TEXT("FPrimeNumberThread Thread is Over !!"));
}


INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogSnippetAsync, Display, TEXT("Hello World! by David !!"));

	// Test_SimpleThread();
	// Test_Event();
	// Test_SimpleQueuedWorker();
	// Test_SimpleTask_1();
	// Test_SimpleTask_2(true);
	// Test_SimpleTask_2(false);
	// Test_SimpleTask_3();

	
	// Test_GraphTask_Simple();
	// Test_GraphTask_Simple1();
	// Test_GraphTask_Simple2();
	// Test_GraphTask_Simple2_Funciton();
	// Test_GraphTask_Simple3();
	// Test_GraphTask_NullTask();
	// Test_GraphTask_ReturnTask();
	// Test_GraphTask_Delegate();
	// Test_GraphTask_Function();

	// Test_FuturePromise();
	// Test_FuturePromise2();
	// Test_FuturePromise3();
	// Test_Async1();
	// Test_Async2();
	Test_Parallel();
	
	
	// TestRNGThread();
	// TestPrimeNumberThreads();
	// Test_FindPrimeNumbers();

	// Test_OpenMP();
	// Test_LockFree_LIFO();
	// Test_LockFree_FIFO();

	// Test_Queue();
	// Test_MPMC();
	// Test_DoubleBuffer();

	// FPlatformProcess::Sleep(60);
	std::cin.get();
	return 0;
}
