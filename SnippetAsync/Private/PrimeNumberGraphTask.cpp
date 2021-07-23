#include "PrimeNumberGraphTask.h"

TArray<int32> FPrimeNumberTask::PrimeNumbers;
FCriticalSection FPrimeNumberTask::PrimeNumbersMutex;
FGraphEventArray FPrimeNumberTask::CompletionEvents;

int32 FPrimeNumberTask::FindNextPrimeNumber()
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
