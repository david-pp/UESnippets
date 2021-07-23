#pragma once

#include "Core.h"

//~~~~~ Multi Threading ~~~
class FPrimeNumberThread : public FRunnable
{
private:
	/** Singleton instance, can access the thread any time via static accessor, if it is active! */
	static FPrimeNumberThread* Runnable;

	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;

	/** The Data Ptr */
	TArray<int32>* PrimeNumbers;


	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;

	//The actual finding of prime numbers
	int32 FindNextPrimeNumber();

	int32 PrimesFoundCount;

	int32 TotalPrimesToFind;

public:
	//Done?
	bool IsFinished() const
	{
		return PrimesFoundCount >= TotalPrimesToFind;
	}

	//~~~ Thread Core Functions ~~~

	//Constructor / Destructor
	FPrimeNumberThread(TArray<int>& TheArray, const int32 IN_PrimesToFindPerTick);

	virtual ~FPrimeNumberThread();

	// Begin FRunnable interface.
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	// End FRunnable interface

	/** Makes sure this thread has stopped properly */
	void EnsureCompletion();

public:
	//~~~ Starting and Stopping Thread ~~~

	/*
	    Start the thread and the worker from static (easy access)!
	    This code ensures only 1 Prime Number thread will be able to run at a time.
	    This function returns a handle to the newly started instance.
	*/
	static FPrimeNumberThread* JoyInit(TArray<int>& TheArray, const int32 IN_TotalPrimesToFind);

	/** Shuts down the thread. Static so it can easily be called from outside the thread context */
	static void Shutdown();

	static bool IsThreadFinished();
};
