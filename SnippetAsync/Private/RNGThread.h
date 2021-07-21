#pragma once

#include "Core.h"

class FRNGThread : public FRunnable
{
public:
	//Constructor
	FRNGThread(int Count = 50000, int minNumber = 0, int maxNumber = 1000, int chunkCount = 20);
	
	//Destructor
	~FRNGThread();

	//Use this method to kill the thread!!
	void EnsureCompletion();

	//Pause the thread 
	void PauseThread();

	//Continue/UnPause the thread
	void ContinueThread();

	//FRunnable interface.
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();

	bool IsThreadPaused();

	FVector GetRandomVector();

private:
	//Thread to run the worker FRunnable on
	FRunnableThread* Thread;

	FCriticalSection m_mutex;
	FEvent* m_semaphore;

	int m_chunkCount;
	int m_amount;
	int m_MinInt;
	int m_MaxInt;

	//As the name states those members are Thread safe
	FThreadSafeBool m_Kill;
	FThreadSafeBool m_Pause;

	//The array is a private member because we don't want to allow anyone from outside the class to access it since we want to ensure a thread safety.
	TArray<FVector> m_RandomVecs;
};
