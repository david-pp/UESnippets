// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBlankObject.h"

#include "BufferArchive.h"
#include "FileHelper.h"
#include "MemoryReader.h"
#include "MyLog.h"
#include "Paths.h"
#include "CoreUObject/Public/CoreUObject.h"


FArchive& operator <<(FArchive& Ar, UMyBlankObject& MyObj)
{
	Ar << MyObj.Health;
	Ar << MyObj.Ammo;
	Ar << MyObj.Location;
	return Ar;
}

void UMyBlankObject::ExampleUObject()
{
	// Case1
	{
		UMyBlankObject* Obj = NewObject<UMyBlankObject>();
		if (Obj)
		{
			Obj->Health = 120.0f;
			Obj->Ammo = 100;
			Obj->Location = FVector(20, 30, 40);

			Obj->SaveData();
		}

		UMyBlankObject* Obj2 = NewObject<UMyBlankObject>();
		if (Obj2)
		{
			Obj2->LoadData();
			Obj2->Dump();
		}
	}

	// Case2
	{
		UMyBlankObject* Obj = NewObject<UMyBlankObject>();
		if (Obj)
		{
			Obj->Health = 240.0f;
			Obj->Ammo = 200;
			Obj->Location = FVector(20, 30, 40);

			Obj->SaveData2();
		}

		UMyBlankObject* Obj2 = NewObject<UMyBlankObject>();
		if (Obj2)
		{
			Obj2->LoadData2();
			Obj2->Dump();
		}
	}
}

void UMyBlankObject::ExampleReflection()
{
	// 获取UMyBlankObject的类型信息
	UClass* MyClass = UMyBlankObject::StaticClass();
	UClass* MyClass2 = ::StaticClass<UMyBlankObject>();

	// 获取UMyEnum的类型信息
	UEnum* MyEnum = ::StaticEnum<EMyEnum>();
	UEnum* MyEnumOld = ::StaticEnum<EMyEnumOld::Type>();
	
	// 获取UMyStruct的类型信息
	UScriptStruct* MyStruct = ::StaticStruct<FMyStruct>();
	UScriptStruct* MyStruct2 = FMyStruct::StaticStruct();
}

bool UMyBlankObject::SaveData()
{
	FBufferArchive BinAr;
	BinAr << *this;

	FString SaveFile = FPaths::ProjectSavedDir() / TEXT("my_blankobj.bin");
	bool Result = FFileHelper::SaveArrayToFile(BinAr, *SaveFile);
	if (!Result)
	{
		UE_LOG(LogMy, Error, TEXT("MyBlankObject Save Data filed: %s"), *SaveFile);
		return false;
	}

	BinAr.FlushCache();
	BinAr.Empty();

	UE_LOG(LogMy, Log, TEXT("MyBlankObject Save Data: %s"), *SaveFile);

	return Result;
}

void UMyBlankObject::LoadData()
{
	TArray<uint8> BinArray;
	FString SaveFile = FPaths::ProjectSavedDir() / TEXT("my_blankobj.bin");
	if (!FFileHelper::LoadFileToArray(BinArray, *SaveFile))
	{
		UE_LOG(LogMy, Error, TEXT("MyBlankObject Load Data filed"))
		return;
	}

	FMemoryReader BinAr(BinArray, true);
	BinAr.Seek(0);
	BinAr << *this;

	BinAr.FlushCache();
	BinArray.Empty();
	BinAr.Close();
}

void UMyBlankObject::Dump()
{
	UE_LOG(LogMy, Log, TEXT("%f, %d, %s"), Health, Ammo, *Location.ToString());
}

bool UMyBlankObject::SaveData2()
{
	FBufferArchive BinAr;
	this->Serialize(BinAr);

	FString SaveFile = FPaths::ProjectSavedDir() / TEXT("my_blankobj2.bin");
	bool Result = FFileHelper::SaveArrayToFile(BinAr, *SaveFile);
	if (!Result)
	{
		UE_LOG(LogMy, Error, TEXT("MyBlankObject Save Data filed: %s"), *SaveFile);
		return false;
	}

	BinAr.FlushCache();
	BinAr.Empty();

	UE_LOG(LogMy, Log, TEXT("MyBlankObject Save Data: %s"), *SaveFile);

	return Result;
}

void UMyBlankObject::LoadData2()
{
	TArray<uint8> BinArray;
	FString SaveFile = FPaths::ProjectSavedDir() / TEXT("my_blankobj2.bin");
	if (!FFileHelper::LoadFileToArray(BinArray, *SaveFile))
	{
		UE_LOG(LogMy, Error, TEXT("MyBlankObject Load Data filed"))
		return;
	}

	FMemoryReader BinAr(BinArray, true);
	BinAr.Seek(0);
	Serialize(BinAr);

	BinAr.FlushCache();
	BinArray.Empty();
	BinAr.Close();
}

void UMyBlankObject::Hello_Callable()
{
}

void UMyBlankObject::Hello_Native_Implementation()
{
}
