#pragma once

#include "Core.h"

class SimpleExampleTask : public FNonAbandonableTask
{
	friend class FAsyncTask<SimpleExampleTask>;

	int32 ExampleData;
	float WorkingTime;

public:
	SimpleExampleTask(int32 InExampleData, float TheWorkingTime = 1)
		: ExampleData(InExampleData), WorkingTime(TheWorkingTime)
	{
	}

	~SimpleExampleTask()
	{
		Log(__FUNCTION__);
	}

	void DoWork()
	{
		// do the work...
		FPlatformProcess::Sleep(WorkingTime);
		Log(__FUNCTION__);
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(ExampleAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	///////////////////////


	void Log(const char* Action)
	{
		uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
		FString CurrentThreadName = FThreadManager::Get().GetThreadName(CurrentThreadId);
		UE_LOG(LogTemp, Display, TEXT("%s[%d] - %s, ExampleData=%d"), *CurrentThreadName, CurrentThreadId,
		       ANSI_TO_TCHAR(Action), ExampleData);
	}
};


inline void Test_SimpleTask_1()
{
	// start an example job, running in GThreadPool
	(new FAutoDeleteAsyncTask<SimpleExampleTask>(1000))->StartBackgroundTask();

	// do an example job now, on this thread
	(new FAutoDeleteAsyncTask<SimpleExampleTask>(2000))->StartSynchronousTask();
}

inline void Test_SimpleTask_2(bool bForceOnThisThread)
{
	//start an example job
	FAsyncTask<SimpleExampleTask>* MyTask = new FAsyncTask<SimpleExampleTask>(1000);

	if (bForceOnThisThread)
		MyTask->StartSynchronousTask();
	else
		MyTask->StartBackgroundTask();

	//to just do it now on this thread
	//Check if the task is done :
	bool IsDone = MyTask->IsDone();
	UE_LOG(LogTemp, Display, TEXT("Is Done : %d"), IsDone);

	//Spinning on IsDone is not acceptable( see EnsureCompletion ), but it is ok to check once a frame.
	//Ensure the task is done, doing the task on the current thread if it has not been started, waiting until completion in all cases.
	MyTask->EnsureCompletion();
	delete MyTask;
}

inline void Test_SimpleTask_3()
{
	using FSimpleExampleAsyncTask = FAsyncTask<SimpleExampleTask>;

	int TaskCount = 20;
	TArray<FSimpleExampleAsyncTask*> Tasks;

	// create a bunch of tasks
	for (int i = 0; i < TaskCount; ++i)
	{
		FSimpleExampleAsyncTask* MyTask = new FSimpleExampleAsyncTask(i + 1, 3);
		if (MyTask)
		{
			MyTask->StartBackgroundTask();
			Tasks.Add(MyTask);
		}
	}

	// Do some other work
	FPlatformProcess::Sleep(5);

	// If not done, cancel it
	TArray<FSimpleExampleAsyncTask*> RemainTasks;
	for (auto Task : Tasks)
	{
		if (Task->IsDone())
		{
			UE_LOG(LogTemp, Display, TEXT("Done .............."));
			delete Task;
		}
		else
		{
			if (Task->Cancel())
			{
				UE_LOG(LogTemp, Display, TEXT("Cancel .............."));
				delete Task;
			}
			else
			{
				UE_LOG(LogTemp, Display, TEXT("Still Working .............."));
				RemainTasks.Add(Task);
			}
		}
	}
	Tasks.Reset();

	// waiting for the remain tasks
	for (auto Task : RemainTasks)
	{
		UE_LOG(LogTemp, Display, TEXT("EnsureCompletion .............."));
		Task->EnsureCompletion();
		delete Task;
	}
	
	UE_LOG(LogTemp, Display, TEXT("Over .............."));
}
