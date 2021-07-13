// Copyright Epic Games, Inc. All Rights Reserved.


#include "SnippetAsync.h"

#include "RequiredProgramMainCPPInclude.h"
#include "Async/Async.h"
#include "Async/AsyncWork.h"
#include <string>

#include "ParallelFor.h"

DEFINE_LOG_CATEGORY_STATIC(LogSnippetAsync, Log, All);

IMPLEMENT_APPLICATION(SnippetAsync, "SnippetAsync");

class FTestWorker : public FRunnable
{
public:
	FTestWorker(const FString& Name) : ActionName(Name)
	{
		RunnableThread = FRunnableThread::Create(this, *Name);
	}

	virtual ~FTestWorker() override
	{
		if (RunnableThread)
		{
			RunnableThread->WaitForCompletion();
			delete RunnableThread;
			RunnableThread = nullptr;
		}
	}

	virtual bool Init() override
	{
		UE_LOG(LogSnippetAsync, Display, TEXT("%s... Init"), *ActionName);
		return true;
	}

	virtual uint32 Run() override
	{
		while (true)
		{
			FPlatformProcess::Sleep(5);
			UE_LOG(LogSnippetAsync, Display, TEXT("%s.. in Thread:%s,%u"), *ActionName,
				*RunnableThread->GetThreadName(), RunnableThread->GetThreadID());
		}
		return 0;
	}

	virtual void Exit() override
	{
		UE_LOG(LogSnippetAsync, Display, TEXT("%s... Exit"), *ActionName);
	}


	FString ActionName;
	FRunnableThread* RunnableThread = nullptr;
};

void TestRunnable()
{
	FTestWorker* Run1 = new FTestWorker(TEXT("RUN1"));
	FTestWorker* Run2 = new FTestWorker(TEXT("RUN2"));
}

class FQueuedWorker : public IQueuedWork
{
public:
	FQueuedWorker(const FString& Name) : WorkerName(Name)
	{
	}

	virtual ~FQueuedWorker() override
	{
		FPlatformProcess::Sleep(0.2);
		uint32 ThreadId = FPlatformTLS::GetCurrentThreadId();
		FString ThreadName = FThreadManager::Get().GetThreadName(ThreadId);
		UE_LOG(LogSnippetAsync, Display, TEXT("%s.. destroyed :%s,%u"), *WorkerName, ThreadId, *ThreadName);
	}

	virtual void DoThreadedWork() override
	{
		FPlatformProcess::Sleep(0.2);
		uint32 ThreadId = FPlatformTLS::GetCurrentThreadId();
		FString ThreadName = FThreadManager::Get().GetThreadName(ThreadId);
		UE_LOG(LogSnippetAsync, Display, TEXT("%s.. in Thread:%s,%u"), *WorkerName, *ThreadName, ThreadId);
	}

	virtual void Abandon() override
	{
	}

	FString WorkerName;
};


void TestThreandPool()
{
	FQueuedThreadPool* Pool = FQueuedThreadPool::Allocate();
	Pool->Create(5, 0, TPri_Normal, TEXT("BlankThreadPool"));

	for (uint32 i = 0; i < 100; ++i)
	{
		FString Name = TEXT("Work") + FString(ANSI_TO_TCHAR(std::to_string(i).c_str()));
		Pool->AddQueuedWork(new FQueuedWorker(Name));
	}
}


class ExampleAsyncTask : public FNonAbandonableTask
{
	friend class FAsyncTask<ExampleAsyncTask>;

	int32 ExampleData;

public:
	ExampleAsyncTask(int32 InExampleData)
		: ExampleData(InExampleData)
	{
	}

	void DoWork()
	{
		uint32 ThreadId = FPlatformTLS::GetCurrentThreadId();
		FString ThreadName = FThreadManager::Get().GetThreadName(ThreadId);
		UE_LOG(LogSnippetAsync, Display, TEXT("%s..Begin in Thread:%s,%u"), TEXT("AsyncTask"), *ThreadName, ThreadId);

		FPlatformProcess::Sleep(5);

		UE_LOG(LogSnippetAsync, Display, TEXT("%s..Done in Thread:%s,%u"), TEXT("AsyncTask"), *ThreadName, ThreadId);
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(ExampleAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}
};

void TestAsyncTask()
{
	//start an example job

	FAsyncTask<ExampleAsyncTask>* MyTask = new FAsyncTask<ExampleAsyncTask>(5);
	MyTask->StartBackgroundTask();

	//--or --

	// MyTask->StartSynchronousTask();

	//to just do it now on this thread
	//Check if the task is done :

	if (MyTask->IsDone())
	{
	}

	//Spinning on IsDone is not acceptable( see EnsureCompletion ), but it is ok to check once a frame.
	//Ensure the task is done, doing the task on the current thread if it has not been started, waiting until completion in all cases.

	MyTask->EnsureCompletion();
	delete MyTask;
}

void TestAsyncTask2()
{
	(new FAutoDeleteAsyncTask<ExampleAsyncTask>(1))->StartBackgroundTask();
}

////////////////////////////////////////////////////

//Multi thread Test, finding prime number
namespace VictoryMultiThreadTest
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~~~
	//   OUTPUT RESULTS OF TASK THREADS
	TArray<int32> PrimeNumbers;
	FCriticalSection PrimeNumbersMutex;

	// This is the array of thread completions to know if all threads are done yet
	FGraphEventArray VictoryMultithreadTest_CompletionEvents;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~~~

	//~~~~~~~~~~~~~~~
	//Are All Tasks Complete?
	//~~~~~~~~~~~~~~~
	bool TasksAreComplete()
	{
		//Check all thread completion events
		for (int32 Index = 0; Index < VictoryMultithreadTest_CompletionEvents.Num(); Index++)
		{
			//If  ! IsComplete()
			if (!VictoryMultithreadTest_CompletionEvents[Index]->IsComplete())
			{
				return false;
			}
		}
		return true;
	}

	//~~~~~~~~~~~
	//Actual Task Code
	//~~~~~~~~~~~
	int32 FindNextPrimeNumber()
	{
		//Last known prime number  + 1
		PrimeNumbersMutex.Lock();
		int32 TestPrime = PrimeNumbers.Last();
		PrimeNumbersMutex.Unlock();

		bool NumIsPrime = false;
		while (!NumIsPrime)
		{
			NumIsPrime = true;

			//Try Next Number
			TestPrime++;

			//Modulus from 2 to current number - 1 
			for (int32 b = 2; b < TestPrime; b++)
			{
				if (TestPrime % b == 0)
				{
					NumIsPrime = false;
					break;
					//~~~
				}
			}
		}

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//Did another thread find this number already?
		PrimeNumbersMutex.Lock();
		bool IsFound = PrimeNumbers.Contains(TestPrime);
		PrimeNumbersMutex.Unlock();

		if (IsFound)
		{
			return FindNextPrimeNumber(); //recursion
		}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		//Success!
		return TestPrime;
	}

	//~~~~~~~~~~~
	//Each Task Thread
	//~~~~~~~~~~~
	class FVictoryTestTask
	{
	public:
		FVictoryTestTask() //send in property defaults here
		{
			//can add properties here
		}

		/** return the name of the task **/
		static const TCHAR* GetTaskName()
		{
			return TEXT("FVictoryTestTask");
		}

		FORCEINLINE static TStatId GetStatId()
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(FVictoryTestTask, STATGROUP_TaskGraphTasks);
		}

		/** return the thread for this task **/
		static ENamedThreads::Type GetDesiredThread()
		{
			return ENamedThreads::AnyThread;
		}

		static ESubsequentsMode::Type GetSubsequentsMode()
		{
			return ESubsequentsMode::TrackSubsequents;
		}

		//~~~~~~~~~~~~~~~~~~~~~~~~
		//Main Function: Do Task!!
		//~~~~~~~~~~~~~~~~~~~~~~~~
		void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
		{
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			PrimeNumbers.Add(FindNextPrimeNumber());

			//***************************************
			//Show Incremental Results in Main Game Thread!

			//  Please note you should not create, destroy, or modify UObjects here.
			//    Do those sort of things after all thread are completed.

			//    All calcs for making stuff can be done in the threads
			//       But the actual making/modifying of the UObjects should be done in main game thread,
			//       which is AFTER all tasks have completed :)

			UE_LOG(LogSnippetAsync, Display, TEXT("A thread completed! ~ %d, thread:%d"), PrimeNumbers.Last(),
				FPlatformTLS::GetCurrentThreadId());
			//***************************************
		}
	};

	//~~~~~~~~~~~~~~~~~~~
	//  Multi-Task Initiation Point 
	//~~~~~~~~~~~~~~~~~~~
	void FindPrimes(const uint32 TotalToFind)
	{
		PrimeNumbers.Empty();
		PrimeNumbers.Add(2);
		PrimeNumbers.Add(3);

		//~~~~~~~~~~~~~~~~~~~~
		//Add thread / task for each of total prime numbers to find
		//~~~~~~~~~~~~~~~~~~~~

		for (uint32 b = 0; b < TotalToFind; b++)
		{
			VictoryMultithreadTest_CompletionEvents.Add(
				TGraphTask<FVictoryTestTask>::CreateTask(NULL, ENamedThreads::GameThread).
				ConstructAndDispatchWhenReady());
			//add properties inside ConstructAndDispatchWhenReady()
		}
	}


	//~~~ In the Game Thread ~~~
	//Starting the Tasks / Threads
	void StartThreadTest()
	{
		FindPrimes(5000); //first 50,000 prime numbers

		while (true)
		{
			if (TasksAreComplete())
			{
				UE_LOG(LogSnippetAsync, Display, TEXT("Multi Thread Test Done!"));
				for (int32 v = 0; v < VictoryMultiThreadTest::PrimeNumbers.Num(); v++)
				{
					UE_LOG(LogSnippetAsync, Display, TEXT("%d ~ %d"), v, PrimeNumbers[v]);
				}
				break;
			}

			FPlatformProcess::Sleep(0.1);
		}
	}


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

				UE_LOG(LogSnippetAsync, Display, TEXT("sum ~ %d, thread:%d,%s"), Sum,
					FPlatformTLS::GetCurrentThreadId(), *ThreadName);
			});
	}
}


/////////////////////////////////////////////////

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogSnippetAsync, Display, TEXT("Hello World! by David !!"));
	// TestRunnable();
	// TestThreandPool();
	// TestAsyncTask2();

	FThreadManager::Get().ForEachThread([](uint32 ThreadID, FRunnableThread* Thread)
		{
			UE_LOG(LogSnippetAsync, Display, TEXT("thread:%s,%u"), *Thread->GetThreadName(), ThreadID);
		});

	// VictoryMultiThreadTest::StartThreadTest();
	VictoryMultiThreadTest::TestParallel();

	FPlatformProcess::Sleep(60);
	return 0;
}
