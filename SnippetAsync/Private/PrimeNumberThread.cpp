#include "PrimeNumberThread.h"

//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//      This line is essential! Compiler error without it
FPrimeNumberThread* FPrimeNumberThread::Runnable = NULL;
//***********************************************************

FPrimeNumberThread::FPrimeNumberThread(TArray<int>& TheArray, const int32 IN_TotalPrimesToFind)
	: TotalPrimesToFind(IN_TotalPrimesToFind)
	  , StopTaskCounter(0)
	  , PrimesFoundCount(0)
{
	//Link to where data should be stored
	PrimeNumbers = &TheArray;

	Thread = FRunnableThread::Create(this, TEXT("FPrimeNumberThread"), 0, TPri_BelowNormal);
}

FPrimeNumberThread::~FPrimeNumberThread()
{
	delete Thread;
	Thread = NULL;
}

//Init
bool FPrimeNumberThread::Init()
{
	//Init the Data 
	PrimeNumbers->Empty();
	PrimeNumbers->Add(2);
	PrimeNumbers->Add(3);

	UE_LOG(LogTemp, Display, TEXT("Prime Number Thread Started!"));
	return true;
}

//Run
uint32 FPrimeNumberThread::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);

	//While not told to stop this thread 
	//and not yet finished finding Prime Numbers
	while (StopTaskCounter.GetValue() == 0 && !IsFinished())
	{
		PrimeNumbers->Add(FindNextPrimeNumber());
		PrimesFoundCount++;


		UE_LOG(LogTemp, Display, TEXT("Prime Number : %d"), PrimeNumbers->Last());

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//prevent thread from using too many resources
		// FPlatformProcess::Sleep(0.01);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	}

	//Run FPrimeNumberThread::Shutdown() from the timer in Game Thread that is watching
	//to see when FPrimeNumberThread::IsThreadFinished()

	return 0;
}

//stop
void FPrimeNumberThread::Stop()
{
	StopTaskCounter.Increment();
}

FPrimeNumberThread* FPrimeNumberThread::JoyInit(TArray<int32>& TheArray, const int32 IN_TotalPrimesToFind)
{
	//Create new instance of thread if it does not exist
	//      and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FPrimeNumberThread(TheArray, IN_TotalPrimesToFind);
	}

	return Runnable;
}

void FPrimeNumberThread::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

void FPrimeNumberThread::Shutdown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

bool FPrimeNumberThread::IsThreadFinished()
{
	if (Runnable) return Runnable->IsFinished();
	return true;
}

int32 FPrimeNumberThread::FindNextPrimeNumber()
{
	//Last known prime number  + 1
	int32 TestPrime = PrimeNumbers->Last();

	bool NumIsPrime = false;
	while (!NumIsPrime)
	{
		NumIsPrime = true;

		//Try Next Number
		TestPrime++;

		//Modulus from 2 to current number - 1 
		for (int32 b = 2; b < TestPrime; b++)
		{
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			//prevent thread from using too many resources
			//FPlatformProcess::Sleep(0.01);
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			if (TestPrime % b == 0)
			{
				NumIsPrime = false;
				break;
			}
		}
	}

	//Success!
	return TestPrime;
}
