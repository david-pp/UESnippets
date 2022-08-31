// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBlankObject.h"
#include "MyBlankDerivedObject.h"

#include "BufferArchive.h"
#include "FileHelper.h"
#include "MemoryReader.h"
#include "MyLog.h"
#include "Paths.h"
#include "CoreUObject/Public/CoreUObject.h"
#include "CoreUObject/Public/Serialization/Formatters/JsonArchiveInputFormatter.h"
#include "CoreUObject/Public/Serialization/Formatters/JsonArchiveOutputFormatter.h"
#include "CoreUObject/Public/UObject/Object.h"

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

// <https://zhuanlan.zhihu.com/p/61042237>
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


	// 获取当前程序中所有的UClass/UEnum/UStriptStruct信息
	TArray<UObject*> Results;
	GetObjectsOfClass(UMyBlankObject::StaticClass(), Results);
	GetObjectsOfClass(UClass::StaticClass(), Results);
	GetObjectsOfClass(UEnum::StaticClass(), Results);
	GetObjectsOfClass(UScriptStruct::StaticClass(), Results);
	for (auto Obj : Results)
	{
		UClass* Class = Cast<UClass>(Obj);
		if (Class)
		{
			UE_LOG(LogMy, Log, TEXT("UClass : %s"), *Class->GetFName().ToString());
		}

		UEnum* Enum = Cast<UEnum>(Obj);
		if (Enum)
		{
			UE_LOG(LogMy, Log, TEXT("UEnum : %s"), *Enum->GetFName().ToString());
		}

		UScriptStruct* Struct = Cast<UScriptStruct>(Obj);
		if (Struct)
		{
			UE_LOG(LogMy, Log, TEXT("UScriptStruct : %s"), *Struct->GetFName().ToString());
		}
	}

	// 根据类名字获取类定义(名字不带U)
	UClass* MyClass3 = FindObject<UClass>(ANY_PACKAGE, TEXT("MyBlankObject"));
	if (MyClass3)
	{
		UE_LOG(LogMy, Log, TEXT("UMyBlankObject : %s, %s"),
		       *MyClass3->GetFName().ToString(),
		       (MyClass3 == UMyBlankObject::StaticClass() ? TEXT("true") : TEXT("false")));
	}

	// 遍历字段
	{
		// Property
		for (TFieldIterator<FProperty> It(UMyBlankDerivedObject::StaticClass()); It; ++It)
		{
			FProperty* Property = *It;
			UE_LOG(LogMy, Log, TEXT("UMyBlankObject.%s"), *Property->GetName());
		}

		// Function (without super class)
		for (TFieldIterator<UFunction> It(UMyBlankDerivedObject::StaticClass(), EFieldIteratorFlags::ExcludeSuper); It;
		     ++It)
		{
			UFunction* Function = *It;
			UE_LOG(LogMy, Log, TEXT("UMyBlankObject.%s(), ParasNum=%d"), *Function->GetFName().ToString(),
			       Function->NumParms);
		}

		// 实现的接口
		for (FImplementedInterface& Interface : UMyBlankDerivedObject::StaticClass()->Interfaces)
		{
			UE_LOG(LogMy, Log, TEXT("Interface Class :%s"), *Interface.Class->GetFName().ToString());
		}

		// 遍历枚举
		// UEnum* MyEnum = StaticEnum<EMyEnum>();
		if (MyEnum)
		{
			for (int I = 0; I < MyEnum->NumEnums(); ++I)
			{
				UE_LOG(LogMy, Log, TEXT("EMyEnum.%s = %d"),
				       *MyEnum->GetNameByIndex(I).ToString(),
				       MyEnum->GetValueByIndex(I));
			}
		}

		// 遍历元数据
#if WITH_METADATA
		UE_LOG(LogMy, Log, TEXT("EMyEnum-Meta ...."));
		UMetaData* MetaData = MyEnum->GetOutermost()->GetMetaData();
		TMap<FName, FString>* Metas = UMetaData::GetMapForObject(MyEnum);
		if (Metas)
		{
			for (auto & Pair : *Metas)
			{
				FName Key = Pair.Key;
				FString Value = Pair.Value;
				UE_LOG(LogMy, Log, TEXT("EMyEnum-Meta: %s-%s"), *Key.ToString(), *Value);
			}
		}
#else
		UE_LOG(LogMy, Log, TEXT("EMyEnum-Meta - without ...."));
#endif
	}

	// 查看继承关系
	{
		// 遍历继承关系
		TArray<FString> SuperClassNames;
		UClass* SuperClass = UMyBlankDerivedObject::StaticClass()->GetSuperClass();
		while (SuperClass)
		{
			SuperClassNames.Add(SuperClass->GetName());
			SuperClass = SuperClass->GetSuperClass();
		}

		UE_LOG(LogMy, Log, TEXT("SuperClasss: UMyBlankDerivedObject->%s"), *FString::Join(SuperClassNames, TEXT("->")));

		// 查看所有子类
		TArray<UClass*> Classes;
		GetDerivedClasses(UMyBlankObject::StaticClass(), Classes);
		for (auto Class : Classes)
		{
			UE_LOG(LogMy, Log, TEXT("MyBlankObject - Derived: %s"), *Class->GetName());
		}
	}

	// 根据名获取Property & Function
	FProperty* HealthProp = UMyBlankDerivedObject::StaticClass()->FindPropertyByName(FName(TEXT("Health")));
	if (HealthProp)
	{
		UE_LOG(LogMy, Log, TEXT("HealProp = %s"), *HealthProp->GetName());
	}

	UFunction* HelloBPFunc = UMyBlankDerivedObject::StaticClass()->FindFunctionByName(FName(TEXT("Hello_Callable")));
	if (HelloBPFunc)
	{
		UE_LOG(LogMy, Log, TEXT("HelloFunc = %s"), *HelloBPFunc->GetFName().ToString());
	}

	UMyBlankDerivedObject* MyBlankDerivedObject = NewObject<UMyBlankDerivedObject>();

	// 获取和设置属性
	{
		// Set methods - 1
		float* HealthPtr = HealthProp->ContainerPtrToValuePtr<float>(MyBlankDerivedObject);
		*HealthPtr = 1111.0;


		// Set methods - 2
		FFloatProperty* HealthFloatProp = CastField<FFloatProperty>(HealthProp);
		if (HealthFloatProp)
		{
			HealthFloatProp->SetPropertyValue(&MyBlankDerivedObject->Health, 2222.0f);
		}

		// Get methods - 1
		float Health1 = 0;
		Health1 = *HealthPtr;

		// Get methods - 2
		float Health2 = 0;
		if (HealthFloatProp)
		{
			Health2 = HealthFloatProp->GetPropertyValue(&MyBlankDerivedObject->Health);
		}

		UE_LOG(LogMy, Log, TEXT("Health = %f,%f,%f, HealthPropertyClass:%s"),
		       MyBlankDerivedObject->Health, Health1, Health2,
		       *HealthProp->GetClass()->GetName());
	}

	// 调用函数
	{
		// Hello_Callable() - 无参数
		MyBlankDerivedObject->ProcessEvent(HelloBPFunc, nullptr);

		// Hello(FString) - 有参数
		UFunction* HelloFunc = UMyBlankDerivedObject::StaticClass()->FindFunctionByName(TEXT("Hello"));
		if (HelloFunc)
		{
			// Method1 - 直接用
			struct HelloParams
			{
				FString Value;
			};
			HelloParams Params;
			Params.Value = TEXT("Unreal");
			MyBlankDerivedObject->ProcessEvent(HelloFunc, &Params);
		}


		// Method2 - 封装下
		auto InvokeHello = [](UMyBlankObject* Object, FString Value)
		{
			UFunction* HelloFunc = UMyBlankDerivedObject::StaticClass()->FindFunctionByName(TEXT("Hello"));
			if (HelloFunc)
			{
				struct HelloParams
				{
					FString Value;
				};
				HelloParams Params;
				Params.Value = Value;
				Object->ProcessEvent(HelloFunc, &Params);
			}
		};

		InvokeHello(MyBlankDerivedObject, TEXT("Unreal!!"));

		// Method3 - 更加通用的封装(TODO: not work)
		// TTuple<> Returns;
		FString Param1 = TEXT("Unreal !!!");
		// InvokeFunction(UMyBlankDerivedObject::StaticClass(), MyBlankDerivedObject, HelloFunc, Returns, Param1);
	}
}

void UMyBlankObject::Serialize2Json(FStructuredArchive& Ar)
{
	FStructuredArchive::FRecord RootRecord = Ar.Open().EnterRecord();

	RootRecord << SA_VALUE(TEXT("Health"), Health);
	RootRecord << SA_VALUE(TEXT("Ammo"), Ammo);
	FStructuredArchive::FRecord PosRecord = RootRecord.EnterField(SA_FIELD_NAME(TEXT("Position"))).EnterRecord();
	PosRecord
		<< SA_VALUE(TEXT("X"), Location.X)
		<< SA_VALUE(TEXT("Y"), Location.Y)
		<< SA_VALUE(TEXT("Z"), Location.Z);

	FStructuredArchive::FRecord StructRecord = RootRecord.EnterField(SA_FIELD_NAME(TEXT("StructVar"))).
	                                                      EnterRecord();
	StructRecord << SA_VALUE(TEXT("Value"), StructVar.Value);
}

void UMyBlankObject::ExampleSerialization2Json()
{
	FString SaveFile = FPaths::ProjectSavedDir() / TEXT("MyObject.json");
	UE_LOG(LogMy, Log, TEXT("Save To: %s"), *SaveFile);

	// Save
	{
		TUniquePtr<FArchive> OutputAr(IFileManager::Get().CreateFileWriter(*SaveFile));
		FJsonArchiveOutputFormatter Formatter(*OutputAr.Get());
		FStructuredArchive Ar(Formatter);

		UMyBlankObject* Obj = NewObject<UMyBlankObject>();
		if (Obj)
		{
			Obj->Health = 120.0f;
			Obj->Ammo = 100;
			Obj->Location = FVector(20, 30, 40);
			Obj->Serialize2Json(Ar);
		}

		Ar.Close();
	}

	// Load
	{
		TUniquePtr<FArchive> OutputAr(IFileManager::Get().CreateFileReader(*SaveFile));
		FJsonArchiveInputFormatter Formatter(*OutputAr.Get());
		FStructuredArchive Ar(Formatter);

		UMyBlankObject* Obj = NewObject<UMyBlankObject>();
		if (Obj)
		{
			Obj->Serialize2Json(Ar);
			Obj->Dump();
		}

		Ar.Close();
	}


	// Engine模块下
#if 0
	FMyStruct MyStruct;
	MyStruct.Value = 100;
	TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(&MyStruct);
	if (JsonObject)
	{
	}

	FString JsonStr;
	if (FJsonObjectConverter::UStructToFormattedJsonObjectString<TCHAR, TPrettyJsonPrintPolicy>(
		FMyStruct::StaticStruct(), &MyStruct, JsonStr))
	{
		UE_LOG(LogMy, Log, TEXT("Json:%s"), *JsonStr);
	}

#endif
}

void UMyBlankObject::ExampleStructFieldProperty()
{
	for (TFieldIterator<FProperty> It(UMyBlankDerivedObject::StaticClass()); It; ++It)
	{
		FProperty* Property = *It;
		UE_LOG(LogMy, Log, TEXT("UMyBlankObject.%s"), *Property->GetName());


		FStructProperty* StructProp = CastField<FStructProperty>(Property);
		// if (StructProp && StructProp->GetFName() == FMyStruct::StaticStruct()->GetFName())
		if (StructProp && StructProp->Struct == FMyStruct::StaticStruct())
		{
			UE_LOG(LogMy, Log, TEXT("UMyBlankObject.%s is FMyStruct"), *Property->GetName());
		}
	}
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
	UE_LOG(LogMy, Log, TEXT("Object: %f, %d, %s"), Health, Ammo, *Location.ToString());
	UE_LOG(LogMy, Log, TEXT("StrutVar.Value = %f"), StructVar.Value);
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


void UMyBlankObject::Hello(FString Value)
{
	UE_LOG(LogMy, Log, TEXT("Hello - %s"), *Value);
}

void UMyBlankObject::Hello_Callable()
{
	UE_LOG(LogMy, Log, TEXT("Hello_Callable"));
}

void UMyBlankObject::Hello_Native_Implementation()
{
}
