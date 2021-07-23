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

inline void Test_SimpleThread()
{
	// Create Threads
	FSimpleThread* SimpleThread1 = new FSimpleThread(TEXT("SimpleThread1"));
	FSimpleThread* SimpleThread2 = new FSimpleThread(TEXT("SimpleThread2"));

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