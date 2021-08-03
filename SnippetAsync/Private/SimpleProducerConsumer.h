#pragma once

#include "Core.h"
#include <atomic>

//
// Tests: TaskGraphTask.cpp
//

//
// TQueue Examples
//
struct FMyItem
{
	FMyItem(uint32 TheId = 0, const FString& TheName = TEXT("Item"))
		: Id(TheId), Name(TheName)
	{
	}

	uint32 Id;
	FString Name;
};

static TQueue<FMyItem, EQueueMode::Spsc> ItemsQueue;

inline void Test_Queue()
{
	// Single Producer
	Async(EAsyncExecution::Thread, []()
	{
		for (uint32 Id = 1; ; Id++)
		{
			FPlatformProcess::Sleep(1);
			ItemsQueue.Enqueue(FMyItem(Id, "Item"));
			UE_LOG(LogTemp, Display, TEXT("Produce: %d,%s"), Id, TEXT("Item"));
		}
	});

	// Single Consumer
	Async(EAsyncExecution::Thread, []()
	{
		while (true)
		{
			if (!ItemsQueue.IsEmpty())
			{
				FMyItem Item;
				ItemsQueue.Dequeue(Item);
				UE_LOG(LogTemp, Display, TEXT("Consume: %d,%s"), Item.Id, *Item.Name);
			}
		}
	});
}

//
// Multiple Producer, Multiple Consumer Examples
//
template <typename ItemType, int QueueSize>
class TQueueMPMC
{
public:
	TQueueMPMC()
	{
		ItemsCount = 0;
		Items.Reserve(QueueSize);
	}

	void Enqueue(const ItemType& Item)
	{
		// Block if full
		if (ItemsCount == QueueSize)
		{
			UE_LOG(LogTemp, Display, TEXT("Enque-Waiting...."));
			FullEvent->Wait();
		}

		// Push
		ItemsMutex.Lock();
		Items.Push(Item);
		ItemsCount = Items.Num();
		ItemsMutex.Unlock();

		// Notify is not empty anymore
		if (ItemsCount >= 1)
		{
			EmptyEvent->Trigger();
		}
	}

	ItemType Dequeue()
	{
		// Block if empty
		if (ItemsCount == 0)
		{
			UE_LOG(LogTemp, Display, TEXT("Dequeue-Waiting...."));
			EmptyEvent->Wait();
		}

		// Pop Item
		ItemType Item;
		ItemsMutex.Lock();
		if (Items.Num() > 0) { Item = Items.Pop(); } else { Item = ItemType(); }
		ItemsCount = Items.Num();
		ItemsMutex.Unlock();

		// Notify is not full anymore
		if (ItemsCount == (QueueSize - 1))
			FullEvent->Trigger();

		return Item;
	}

private:
	FEventRef FullEvent;
	FEventRef EmptyEvent;
	TAtomic<int> ItemsCount;
	FCriticalSection ItemsMutex;
	TArray<ItemType> Items;
};

struct MPMCTest
{
	static TQueueMPMC<int, 10> QueueMPMC;

	static void Producer(int ItemID = 0)
	{
		int Item = ItemID;
		while (true)
		{
			// produce item
			Item++;
			// enqueue, if is full, waiting for consumer
			QueueMPMC.Enqueue(Item);
		}
	}

	static void Consumer()
	{
		uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
		FString CurrentThreadName = FThreadManager::Get().GetThreadName(CurrentThreadId);

		while (true)
		{
			// dequeue, if is empty, waiting for producer
			int Item = QueueMPMC.Dequeue();
			// consume item
			UE_LOG(LogTemp, Display, TEXT("Consumer@%s, Item=%d"), *CurrentThreadName, Item);
		}
	}
};

inline void Test_MPMC()
{
	const int NumProducer = 5;
	const int NumConsumer = 3;

	// Multiple Producer
	for (int i = 0; i < NumProducer; ++i)
	{
		Async(EAsyncExecution::Thread, [i]()
		{
			MPMCTest::Producer(i * 1000000);
		});
	}

	// Multiple Consumer
	for (int i = 0; i < NumConsumer; ++i)
	{
		Async(EAsyncExecution::Thread, &MPMCTest::Consumer);
	}
}

//
// Double Buffering Examples
//
template <typename ItemType>
class TDoubleBuffer
{
public:
	TDoubleBuffer(uint32 Capacity = (uint32)-1)
		: MaxCapacity(Capacity)
	{
		WriteBuffer = new TArray<ItemType>();
		ReadBuffer = new TArray<ItemType>();
	}

	~TDoubleBuffer()
	{
		delete WriteBuffer;
		delete ReadBuffer;
	}

	bool Enqueue(const ItemType& Item)
	{
		FScopeLock Lock(&SwapMutex);
		if ((uint32)WriteBuffer->Num() > MaxCapacity)
			return false;

		WriteBuffer->Push(Item);
		return true;
	}

	bool Dequeue(ItemType& Item)
	{
		if (ReadBuffer->Num() == 0)
		{
			// Swap Read&Write Buffer
			FScopeLock Lock(&SwapMutex);
			Swap(WriteBuffer, ReadBuffer);
			if (ReadBuffer->Num() == 0)
				return false;
		}

		if (ReadBuffer->Num() > 0)
		{
			Item = ReadBuffer->Pop();
			return true;
		}

		return false;
	}

private:
	uint32 MaxCapacity;
	FCriticalSection SwapMutex;
	TArray<ItemType>* WriteBuffer;
	TArray<ItemType>* ReadBuffer;
};

inline void Test_DoubleBuffer()
{
	TDoubleBuffer<uint32> DoubleBuffer;

	std::atomic<bool> bStop{false};
	// Producer Thread
	auto IOThread = Async(EAsyncExecution::Thread, [&bStop, &DoubleBuffer]()
	{
		FRandomStream Rand;
		Rand.GenerateNewSeed();
		while (!bStop)
		{
			// Produce a Object
			DoubleBuffer.Enqueue(Rand.GetUnsignedInt());
		}
	});

	// Consumer Thread(Main)
	const uint32 MaxConsuming = 100000;
	for (uint32 i = 0; i < MaxConsuming;)
	{
		uint32 Item;
		if (DoubleBuffer.Dequeue(Item))
		{
			++i;
			UE_LOG(LogTemp, Display, TEXT("Consumer %u, Item=%u"), i, Item);
		}
	}

	bStop = true;
	IOThread.Wait();

	UE_LOG(LogTemp, Display, TEXT("OVer...."));
}
