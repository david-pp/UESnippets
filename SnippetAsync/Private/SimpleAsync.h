#pragma once

#include "Core.h"

inline void AsyncLog(const char* Action)
{
	int32 ThreadId = FPlatformTLS::GetCurrentThreadId();
	FString ThreadName = FThreadManager::Get().GetThreadName(ThreadId);
	UE_LOG(LogTemp, Display, TEXT("%s[%d], %s"), *ThreadName, ThreadId, ANSI_TO_TCHAR(Action));
}

// Future & Promise
inline void Test_FuturePromise()
{
	TPromise<bool> Promise;
	TFuture<bool> Future = Promise.GetFuture();

	// running in any threads
	FFunctionGraphTask::CreateAndDispatchWhenReady([&Promise]()
	{
		FPlatformProcess::Sleep(3);

		AsyncLog("do the promise");
		Promise.SetValue(true);
	});

	// waiting for the promise
	UE_LOG(LogTemp, Display, TEXT("waiting for the promise..."));
	// Future.Wait();
	Future.WaitFor(FTimespan::FromSeconds(5));
	// Future.WaitUntil(FDateTime(2022, 1, 1));

	if (Future.IsReady())
	{
		UE_LOG(LogTemp, Display, TEXT("keep the promise, future is %d"), Future.Get());
	}
	else
	{
		Promise.SetValue(false);
		UE_LOG(LogTemp, Display, TEXT("break the promise"));
	}
}

/////////////////////////////////////////////////////////////////////////////////

inline void Test_FuturePromise2()
{
	// promise with callback
	TPromise<bool> Promise([]()
	{
		AsyncLog("the promise is set !");
	});

	TFuture<bool> Future = Promise.GetFuture();

	// running in any threads
	FFunctionGraphTask::CreateAndDispatchWhenReady([&Promise]()
	{
		FPlatformProcess::Sleep(3);

		AsyncLog("do the promise");
		Promise.SetValue(true);
	})->Wait(ENamedThreads::GameThread);
}

TFuture<int> DoSthAsync(int Value)
{
	TPromise<int> Promise;
	TFuture<int> Future = Promise.GetFuture();

	class FGraphTaskSimple
	{
	public:
		// CAUTION!: Must not use references in the constructor args; use pointers instead if you need by reference
		FGraphTaskSimple(TPromise<int>&& ThePromise, int TheValue)
			: Promise(MoveTemp(ThePromise)), FutureValue(TheValue)
		{
		}

		FORCEINLINE TStatId GetStatId() const
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(FGraphTaskSimple, STATGROUP_TaskGraphTasks);
		}

		static ENamedThreads::Type GetDesiredThread()
		{
			return ENamedThreads::AnyThread;
		}

		static ESubsequentsMode::Type GetSubsequentsMode()
		{
			return ESubsequentsMode::FireAndForget;
		}

		void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
		{
			AsyncLog("DoSthAsync.... Begin");
			FPlatformProcess::Sleep(3);
			AsyncLog("DoSthAsync.... Done");

			Promise.SetValue(FutureValue);
		}

	private:
		TPromise<int> Promise;
		int FutureValue;
	};

	TGraphTask<FGraphTaskSimple>::CreateTask().ConstructAndDispatchWhenReady(MoveTemp(Promise), MoveTemp(Value));

	return MoveTemp(Future);
}

inline void Test_FuturePromise3()
{
	// usage1: get
	{
		auto Future = DoSthAsync(100);

		// todo other ...
		FPlatformProcess::Sleep(1);
		UE_LOG(LogTemp, Display, TEXT("Do something else .."));

		// waiting for the value
		int Value = Future.Get();
		UE_LOG(LogTemp, Display, TEXT("Value = %d"), Value);
	}

	// usage2 : then
	DoSthAsync(1)
			.Then([](TFuture<int> Future) -> int
			{
				AsyncLog("then1 .. ");
				UE_LOG(LogTemp, Display, TEXT("Value = %d"), Future.Get());
				return 2;
			})
			.Then([](TFuture<int> Future)
			{
				AsyncLog("then2 .. ");
				UE_LOG(LogTemp, Display, TEXT("Value = %d"), Future.Get());
			})
			.Wait();

	AsyncLog("Over....");
}

/////////////////////////////////////////////////////////////////////////////////

int SimpleAsyncFunc()
{
	AsyncLog("SimpleAsyncFunc .... Begin");
	FPlatformProcess::Sleep(1);
	AsyncLog("SimpleAsyncFunc .... Over");
	return 123;
}

inline void Test_Async1()
{
	auto Future = Async(EAsyncExecution::TaskGraph, SimpleAsyncFunc);

	// todo other ...
	FPlatformProcess::Sleep(1);
	UE_LOG(LogTemp, Display, TEXT("Do something else .."));

	// waiting for the value
	int Value = Future.Get();
	UE_LOG(LogTemp, Display, TEXT("Value = %d"), Value);
}

inline void Test_Async2()
{
	// using global function
	Async(EAsyncExecution::Thread, SimpleAsyncFunc);

	// using task
	// TUniqueFunction<int()> Task = SimpleTestFunc;
	TUniqueFunction<int()> Task = []()
	{
		AsyncLog("Lambda1 .... Begin");
		FPlatformProcess::Sleep(1);
		AsyncLog("Lambda1 .... Over");
		return 123;
	};

	Async(EAsyncExecution::Thread, MoveTemp(Task));

	// using inline lambda
	Async(EAsyncExecution::Thread, []()
	{
		AsyncLog("Inline Lambda .... Begin");
		FPlatformProcess::Sleep(1);
		AsyncLog("Inline Lambda .... Over");
	});

	// with completion callback
	Async(EAsyncExecution::ThreadPool,
	      []()
	      {
		      AsyncLog("Inline Lambda2 .... Begin");
		      FPlatformProcess::Sleep(1);
		      AsyncLog("Inline Lambda2 .... Over");
	      },
	      []()
	      {
		      AsyncLog("Inline Lambda2 .... Completion Callback");
	      });

	// Execute a given function asynchronously using a separate thread.
	AsyncThread([]()
	{
		AsyncLog("AsyncThread Function .... Begin");
		FPlatformProcess::Sleep(1);
		AsyncLog("AsyncThread Function .... Over");
	});

	// Execute a given function asynchronously on the specified thread pool.
	AsyncPool(*GThreadPool, []()
	{
		AsyncLog("AsyncPool Function .... Begin");
		FPlatformProcess::Sleep(1);
		AsyncLog("AsyncPool Function .... Over");
	});

	// Executing code asynchronously on the Task Graph.
	AsyncTask(ENamedThreads::AnyThread, []()
	{
		AsyncLog("AsyncTask Function .... Begin");
		FPlatformProcess::Sleep(1);
		AsyncLog("AsyncTask Function .... Over");
	});
}

/////////////////////////////////////////////////////////////////////////////////

inline void Test_Parallel()
{
	// parallel
	ParallelFor(100, [](int32 Index)
	{
		int32 ThreadId = FPlatformTLS::GetCurrentThreadId();
		FString ThreadName = FThreadManager::Get().GetThreadName(ThreadId);
		UE_LOG(LogTemp, Display, TEXT("%s[%d],Parallel Task, Index:%d"), *ThreadName, ThreadId, Index);	
	});

	// done
	UE_LOG(LogTemp, Display, TEXT("Over...."));	
}