#pragma once
#include "Core.h"


class FGraphTaskSimple
{
public:
	// CAUTION!: Must not use references in the constructor args; use pointers instead if you need by reference
	FGraphTaskSimple(const TCHAR* TheName, int InSomeArgument, float InWorkingTime = 1.0f)
		: TaskName(TheName), SomeArgument(InSomeArgument), WorkingTime(InWorkingTime)
	{
		// Usually the constructor doesn't do anything except save the arguments for use in DoWork or GetDesiredThread.
		Log(__FUNCTION__);
	}

	~FGraphTaskSimple()
	{
		// you will be destroyed immediately after you execute. Might as well do cleanup in DoWork, but you could also use a destructor.
		Log(__FUNCTION__);
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
		// The arguments are useful for setting up other tasks. 
		// Do work here, probably using SomeArgument.
		FPlatformProcess::Sleep(WorkingTime);
		Log(__FUNCTION__);

		// MyCompletionGraphEvent->DontCompleteUntil(
		// TGraphTask<FSomeChildTask>::CreateTask(NULL, CurrentThread).ConstructAndDispatchWhenReady());
	}

public:
	FString TaskName;

	int SomeArgument;

	float WorkingTime;

	void Log(const char* Action)
	{
		uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
		FString CurrentThreadName = FThreadManager::Get().GetThreadName(CurrentThreadId);
		UE_LOG(LogTemp, Display, TEXT("%s@%s[%d] - %s, SomeArgument=%d"), *TaskName, *CurrentThreadName,
		       CurrentThreadId,
		       ANSI_TO_TCHAR(Action), SomeArgument);
	}
};


class FTask : public FGraphTaskSimple
{
public:
	using FGraphTaskSimple::FGraphTaskSimple;

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FGraphTask, STATGROUP_TaskGraphTasks);
	}

	static ENamedThreads::Type GetDesiredThread()
	{
		return ENamedThreads::AnyThread;
	}

	static ESubsequentsMode::Type GetSubsequentsMode()
	{
		return ESubsequentsMode::TrackSubsequents;
	}
};


// Only One Task - Auto delete
inline void Test_GraphTask_Simple()
{
	// construct and dispatch, when the work is done, deleted automatically
	TGraphTask<FGraphTaskSimple>::CreateTask().
		ConstructAndDispatchWhenReady(TEXT("SimpleTask1"), 10000, 3);

	// hold and unlock to run
	TGraphTask<FGraphTaskSimple>* Task = TGraphTask<FGraphTaskSimple>::CreateTask().ConstructAndHold(
		TEXT("SimpleTask2"), 20000);
	if (Task)
	{
		// unlock to run
		UE_LOG(LogTemp, Display, TEXT("Task is Unlock to Run..."));
		Task->Unlock();
		Task = nullptr;
	}
}

// Only One Task - Waiting in gamethread
inline void Test_GraphTask_Simple1()
{
	// task completes after it's waited for
	FGraphEventRef Event = FFunctionGraphTask::CreateAndDispatchWhenReady([]()
		{
			UE_LOG(LogTemp, Display, TEXT("Main task"));
			FPlatformProcess::Sleep(5.f); // pause for a bit to let waiting start
		}
	);
	check(!Event->IsComplete());
	Event->Wait(ENamedThreads::GameThread);
	UE_LOG(LogTemp, Display, TEXT("Over1 ..."));

	// tasks
	FGraphEventArray Tasks;
	for (int i = 0; i < 10; ++i)
	{
		Tasks.Add(FFunctionGraphTask::CreateAndDispatchWhenReady([i]()
		{
			UE_LOG(LogTemp, Display, TEXT("Task %d"), i);
		}));
	}
	FTaskGraphInterface::Get().WaitUntilTasksComplete(MoveTemp(Tasks), ENamedThreads::GameThread);
	UE_LOG(LogTemp, Display, TEXT("Over2 ..."));
}


// TaskA -> TaskB -> TaskC
inline void Test_GraphTask_Simple2()
{
	UE_LOG(LogTemp, Display, TEXT("Start ......"));

	FGraphEventRef TaskA, TaskB, TaskC;

	// TaskA
	{
		TaskA = TGraphTask<FTask>::CreateTask().ConstructAndDispatchWhenReady(TEXT("TaksA"), 1, 3);
	}

	// TaskA -> TaskB
	{
		FGraphEventArray Prerequisites;
		Prerequisites.Add(TaskA);
		TaskB = TGraphTask<FTask>::CreateTask(&Prerequisites).ConstructAndDispatchWhenReady(TEXT("TaksB"), 2, 2);
	}


	// TaskB -> TaskC
	{
		FGraphEventArray Prerequisites{TaskB};
		TaskC = TGraphTask<FTask>::CreateTask(&Prerequisites).ConstructAndDispatchWhenReady(TEXT("TaksC"), 3, 1);
	}


	UE_LOG(LogTemp, Display, TEXT("Construct is Done ......"));


	UE_LOG(LogTemp, Display, TEXT("TaskA is Done : %d"), TaskA->IsComplete());
	UE_LOG(LogTemp, Display, TEXT("TaskB is Done : %d"), TaskA->IsComplete());
	UE_LOG(LogTemp, Display, TEXT("TaskC is Done : %d"), TaskC->IsComplete());

	// Waiting until TaskC is Done
	// FTaskGraphInterface::Get().WaitUntilTaskCompletes(TaskC);
	// Or.
	TaskC->Wait();

	UE_LOG(LogTemp, Display, TEXT("TaskA is Done : %d"), TaskA->IsComplete());
	UE_LOG(LogTemp, Display, TEXT("TaskB is Done : %d"), TaskA->IsComplete());
	UE_LOG(LogTemp, Display, TEXT("TaskC is Done : %d"), TaskC->IsComplete());

	UE_LOG(LogTemp, Display, TEXT("Over ......"));
}

inline void Test_GraphTask_Simple2_Funciton()
{
	UE_LOG(LogTemp, Display, TEXT("Start ......"));

	auto TaskA = FFunctionGraphTask::CreateAndDispatchWhenReady(
		[](ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
		{
			auto TaskB = FFunctionGraphTask::CreateAndDispatchWhenReady(
				[](ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
				{
					auto TaskC = FFunctionGraphTask::CreateAndDispatchWhenReady(
						[](ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
						{
							FPlatformProcess::Sleep(1);
							UE_LOG(LogTemp, Display, TEXT("TaskC is Done"));
						});

					FPlatformProcess::Sleep(2);
					UE_LOG(LogTemp, Display, TEXT("TaskB is Done"));
				});

			FPlatformProcess::Sleep(1);
			UE_LOG(LogTemp, Display, TEXT("TaskA is Done"));
		});

	TaskA->Wait(ENamedThreads::GameThread);
	UE_LOG(LogTemp, Display, TEXT("Over ......"));
}

//
//        -> TaskB -> TaskC ->
//       /                    \
// TaskA -> TaskD ---------------> TaskE
//       
inline void Test_GraphTask_Simple3()
{
	FGraphEventRef TaskA, TaskB, TaskC, TaskD, TaskE;

	TaskA = TGraphTask<FTask>::CreateTask().ConstructAndDispatchWhenReady(TEXT("TaksA"), 1, 1);

	{
		FGraphEventArray Prerequisites;
		Prerequisites.Add(TaskA);
		TaskB = TGraphTask<FTask>::CreateTask(&Prerequisites).ConstructAndDispatchWhenReady(TEXT("TaksB"), 1, 1);
	}

	{
		FGraphEventArray Prerequisites;
		Prerequisites.Add(TaskB);
		TaskC = TGraphTask<FTask>::CreateTask(&Prerequisites).ConstructAndDispatchWhenReady(TEXT("TaksC"), 1, 1);
	}

	{
		FGraphEventArray Prerequisites;
		Prerequisites.Add(TaskA);
		TaskD = TGraphTask<FTask>::CreateTask(&Prerequisites).ConstructAndDispatchWhenReady(TEXT("TaksD"), 1, 3);
	}

	{
		FGraphEventArray Prerequisites;
		Prerequisites.Add(TaskC);
		Prerequisites.Add(TaskD);
		TaskE = TGraphTask<FTask>::CreateTask(&Prerequisites).ConstructAndDispatchWhenReady(TEXT("TaksE"), 1, 1);
	}

	UE_LOG(LogTemp, Display, TEXT("Construct is Done ......"));

	TaskE->Wait();

	UE_LOG(LogTemp, Display, TEXT("Over ......"));
}


//
//  TaskA ->
//          | -> NullTask(Gather/Fence)
//  TaskB ->
//
inline void Test_GraphTask_NullTask()
{
	auto TaskA = TGraphTask<FTask>::CreateTask().ConstructAndDispatchWhenReady(TEXT("TaskA"), 1, 2);
	auto TaskB = TGraphTask<FTask>::CreateTask().ConstructAndDispatchWhenReady(TEXT("TaskB"), 2, 1);


	FGraphEventRef CompleteEvent;
	{
		DECLARE_CYCLE_STAT(TEXT("FNullGraphTask.Gather"),
		                   STAT_FNullGraphTask_Gather,
		                   STATGROUP_TaskGraphTasks);

		FGraphEventArray Prerequisites;
		Prerequisites.Add(TaskA);
		Prerequisites.Add(TaskB);
		CompleteEvent = TGraphTask<FNullGraphTask>::CreateTask(&Prerequisites).ConstructAndDispatchWhenReady(
			GET_STATID(STAT_FNullGraphTask_Gather), ENamedThreads::GameThread);
	}


	UE_LOG(LogTemp, Display, TEXT("Construct is Done ......"));

	CompleteEvent->Wait();

	UE_LOG(LogTemp, Display, TEXT("Over ......"));
}


// FReturnGraphTask
inline void Test_GraphTask_ReturnTask()
{
	// Stop the RHI Thread (using IsRHIThreadRunning() is unreliable since RT may be stopped)
	if (FTaskGraphInterface::IsRunning() && FTaskGraphInterface::Get().
		IsThreadProcessingTasks(ENamedThreads::RHIThread))
	{
		DECLARE_CYCLE_STAT(TEXT("Wait For RHIThread Finish"), STAT_WaitForRHIThreadFinish, STATGROUP_TaskGraphTasks);
		FGraphEventRef QuitTask = TGraphTask<FReturnGraphTask>::CreateTask(nullptr, ENamedThreads::GameThread).
			ConstructAndDispatchWhenReady(ENamedThreads::RHIThread);
		FTaskGraphInterface::Get().WaitUntilTaskCompletes(QuitTask, ENamedThreads::GameThread_Local);
	}
}

//FTriggerEventGraphTask
// a task that triggers an event(operating system Event object)


// FSimpleDelegateGraphTask
// for simple delegate based tasks.This is less efficient than a custom task, doesn’t provide the task arguments, doesn’t allow specification of the current thread, etc.

// FDelegateGraphTask
// class for more full featured delegate based tasks.Still less efficient than a custom task, but provides all of the args.
inline void Test_GraphTask_Delegate()
{
	// Simple Delegate
	FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
		FSimpleDelegateGraphTask::FDelegate::CreateLambda([]()
		{
			uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
			FString CurrentThreadName = FThreadManager::Get().GetThreadName(CurrentThreadId);
			UE_LOG(LogTemp, Display, TEXT("%s[%d] - Simple Delegate"), *CurrentThreadName, CurrentThreadId);
		}),
		TStatId()
	);


	// Delegate
	FTaskGraphInterface::Get().WaitUntilTaskCompletes(
		FDelegateGraphTask::CreateAndDispatchWhenReady(
			FDelegateGraphTask::FDelegate::CreateLambda(
				[](ENamedThreads::Type InCurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
				{
					FPlatformProcess::Sleep(3);
					uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
					FString CurrentThreadName = FThreadManager::Get().GetThreadName(CurrentThreadId);
					UE_LOG(LogTemp, Display, TEXT("%s[%d] - Delegate, %d"), *CurrentThreadName, CurrentThreadId,
					       InCurrentThread);
				}),
			TStatId()
		),
		ENamedThreads::GameThread
	);

	UE_LOG(LogTemp, Display, TEXT("Over ......"));
}


// FFunctionGraphTask
inline void Test_GraphTask_Function()
{
	FFunctionGraphTask::CreateAndDispatchWhenReady([]()
	{
		uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
		FString CurrentThreadName = FThreadManager::Get().GetThreadName(CurrentThreadId);
		UE_LOG(LogTemp, Display, TEXT("%s[%d] - Fuction with Void"), *CurrentThreadName, CurrentThreadId);
	}, TStatId());


	FFunctionGraphTask::CreateAndDispatchWhenReady(
		[](ENamedThreads::Type InCurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
		{
			FPlatformProcess::Sleep(3);
			uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
			FString CurrentThreadName = FThreadManager::Get().GetThreadName(CurrentThreadId);
			UE_LOG(LogTemp, Display, TEXT("%s[%d] - Function with parameters, %d"), *CurrentThreadName, CurrentThreadId,
			       InCurrentThread);
		}, TStatId())->Wait(ENamedThreads::GameThread);

	UE_LOG(LogTemp, Display, TEXT("Over ......"));
}
