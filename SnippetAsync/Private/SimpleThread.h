#pragma once

#include "Core.h"
#include "Misc/ScopeRWLock.h"

class FSimpleThread : public FRunnable
{
public:
	FSimpleThread(const FString& TheName) : Name(TheName)
	{
		RunnableThread = FRunnableThread::Create(this, *Name);
		Log(__FUNCTION__);
	}

	virtual ~FSimpleThread() override
	{
		if (RunnableThread)
		{
			RunnableThread->WaitForCompletion();
			delete RunnableThread;
			RunnableThread = nullptr;
			Log(__FUNCTION__);
		}
	}

	virtual bool Init() override
	{
		Log(__FUNCTION__);
		return true;
	}

	virtual uint32 Run() override
	{
		while (!bStop)
		{
			FPlatformProcess::Sleep(1);
			Log(__FUNCTION__);
		}
		return 0;
	}

	virtual void Exit() override
	{
		Log(__FUNCTION__);
	}


	virtual void Stop() override
	{
		bStop = true;
		if (RunnableThread)
			RunnableThread->WaitForCompletion();
	}

	void Log(const char* Action)
	{
		uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
		FString CurrentThreadName = FThreadManager::Get().GetThreadName(CurrentThreadId);

		if (RunnableThread)
		{
			UE_LOG(LogTemp, Display, TEXT("%s@%s[%d] - %s,%d, %s"), *Name, *CurrentThreadName, CurrentThreadId,
			       *RunnableThread->GetThreadName(),
			       RunnableThread->GetThreadID(), ANSI_TO_TCHAR(Action));
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("%s@%s[%d] - %s,%d, %s"), *Name, *CurrentThreadName, CurrentThreadId,
			       TEXT("NULL"), 0, ANSI_TO_TCHAR(Action));
		}
	}

public:
	FString Name;
	FRunnableThread* RunnableThread = nullptr;
	FThreadSafeBool bStop;
};


class ThreadSafeArray
{
public:
	int32 GetValue(int32 Index)
	{
		FScopeLock Lock(&CS);
		return Values[Index];
	}

	void AppendValue(int32 Value)
	{
		CS.Lock();
		Values.Add(Value);
		CS.Unlock();
	}

private:
	FCriticalSection CS;
	TArray<int32> Values;
};


class ThreadSafeArray2
{
public:
	int32 GetValue(int32 Index)
	{
		FRWScopeLock ScopeLock(ValuesLock, SLT_ReadOnly);
		return Values[Index];
	}

	void AppendValue(int32 Value)
	{
		ValuesLock.WriteLock();
		Values.Add(Value);
		ValuesLock.WriteUnlock();
	}

private:
	FRWLock ValuesLock;
	TArray<int32> Values;
};


/////////////////////////////////////////////////////////////////////////////////

#define SAFE_DELETE(Ptr)  if (Ptr) { delete Ptr; Ptr = nullptr; }

inline void DumpAllThreads(const char* Log)
{
	FThreadManager::Get().ForEachThread(
		[=](uint32 ThreadID, FRunnableThread* Thread)
		{
			UE_LOG(LogTemp, Display, TEXT("%s: %s,%u"), ANSI_TO_TCHAR(Log), *Thread->GetThreadName(), ThreadID);
		});
}

inline void Test_SimpleThread()
{
	// Create Threads
	FSimpleThread* SimpleThread1 = new FSimpleThread(TEXT("SimpleThread1"));
	FSimpleThread* SimpleThread2 = new FSimpleThread(TEXT("SimpleThread2"));

	DumpAllThreads(__FUNCTION__);

	// Ticks
	int TickCount = 100;
	for (int i = 0; i < TickCount; ++i)
	{
		// Consume
		UE_LOG(LogTemp, Display, TEXT("Tick[%d] ........ "), i);
		FPlatformProcess::Sleep(0.1);
	}

	// Stop Thread
	SimpleThread1->Stop();
	SimpleThread2->Stop();

	// Destroy Threads
	SAFE_DELETE(SimpleThread1);
	SAFE_DELETE(SimpleThread2);
}

inline void Test_Atomic()
{
	TAtomic<int> Counter;
	Counter ++; // Atomic increment -> FPlatformAtomics::InterlockedIncrement
	if (Counter.Load()) // Atomic read -> FPlatformAtomics::AtomicRead
	{
	}

	FThreadSafeCounter Counter2;
	Counter2.Increment(); // FPlatformAtomics::InterlockedIncrement
	Counter2.Decrement(); // FPlatformAtomics::InterlockedDecrement
	if (Counter2.GetValue() == 0) // FPlatformAtomics::AtomicRead
	{
	}
}


inline void Test_Event1()
{
	FEvent* SyncEvent = nullptr;

	Async(EAsyncExecution::Thread, [&SyncEvent]()
	{
		FPlatformProcess::Sleep(3);
		if (SyncEvent)
		{
			SyncEvent->Trigger();
			UE_LOG(LogTemp, Display, TEXT("Trigger ....."));
		}
	});

	SyncEvent = FPlatformProcess::GetSynchEventFromPool(true);
	SyncEvent->Wait((uint32)-1);
	FPlatformProcess::ReturnSynchEventToPool(SyncEvent);

	UE_LOG(LogTemp, Display, TEXT("Over ....."));
}

inline void Test_Event2()
{
	FEventRef SyncEvent(EEventMode::AutoReset);

	FEvent* Event = SyncEvent.operator->();
	Async(EAsyncExecution::Thread, [Event]()
	{
		FPlatformProcess::Sleep(3);
		Event->Trigger();
		UE_LOG(LogTemp, Display, TEXT("Trigger ....."));
	});

	SyncEvent->Wait((uint32)-1);
	UE_LOG(LogTemp, Display, TEXT("Over ....."));
}

inline void Test_Event()
{
	// waiting..
	{
		FScopedEvent SyncEvent;

		Async(EAsyncExecution::Thread, [&SyncEvent]()
		{
			FPlatformProcess::Sleep(3);
			SyncEvent.Trigger();
			UE_LOG(LogTemp, Display, TEXT("Trigger ....."));
		});
	}

	UE_LOG(LogTemp, Display, TEXT("Over ....."));
}

//////////////////////////////////////////////////

// #include <omp.h>

inline void Test_OpenMP()
{
	static long num_rects = 1000000;

	// pi = Int(4/(1+x^2))
	double mid, height, width, sum = 0;
	int i = 0;
	double area = 0;
	width = 1. / (double)num_rects;

#pragma omp parallel for private(mid, height, width) reduce(+:sum)
	for (i = 0; i < num_rects; i++)
	{
		mid = (i + 0.5) * width;
		height = 4.0 / (1. + mid * mid);
		sum += height;
	}

	area = width * sum;
	UE_LOG(LogTemp, Display, TEXT("Pi is : %f"), area);
}


//////////////////////////////////////////////////