#pragma once

#include "Core.h"

class FSimpleQueuedWorker : public IQueuedWork
{
public:
	FSimpleQueuedWorker(const FString& Name) : WorkerName(Name)
	{
		Log(__FUNCTION__);
	}

	virtual ~FSimpleQueuedWorker() override
	{
		Log(__FUNCTION__);
	}

	virtual void DoThreadedWork() override
	{
		FPlatformProcess::Sleep(0.2);
		Log(__FUNCTION__);

		// Finish the task, delete the worker
		delete this;
	}

	virtual void Abandon() override
	{
		Log(__FUNCTION__);

		// Abandon the task, delete the worker
		delete this;
	}

	void Log(const char* Action)
	{
		uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
		FString CurrentThreadName = FThreadManager::Get().GetThreadName(CurrentThreadId);
		UE_LOG(LogTemp, Display, TEXT("%s@%s[%d] - %s"),
		       *WorkerName, *CurrentThreadName, CurrentThreadId, ANSI_TO_TCHAR(Action));
	}

public:
	FString WorkerName;
};


inline void Test_SimpleQueuedWorker()
{
	// Create A Thread Pool
	FQueuedThreadPool* Pool = FQueuedThreadPool::Allocate();
	Pool->Create(5, 0, TPri_Normal, TEXT("SimpleThreadPool"));

	int WokerNum = 100;
	for (int i = 0; i < WokerNum; ++i)
	{
		FString Name = TEXT("Worker") + FString::FromInt(i);

		// Worker will be deleted when the job is done
		Pool->AddQueuedWork(new FSimpleQueuedWorker(Name));
	}

	// Ticks
	int TickCount = 20;
	for (int i = 0; i < TickCount; ++i)
	{
		// Consume
		UE_LOG(LogTemp, Display, TEXT("Tick[%d] ........ "), i);
		FPlatformProcess::Sleep(0.1);
	}

	// Destroy, if work is not done, call Abandon
	Pool->Destroy();
	delete Pool;
}
