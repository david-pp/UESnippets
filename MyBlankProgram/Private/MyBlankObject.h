// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreUObject/Public/UObject/Object.h"
#include "MyInterface.h"
#include "MyStruct.h"
#include "MyEnum.h"
#include "CoreUObject/Public/UObject/UnrealType.h"
#include "MyBlankObject.generated.h"


/**
 * UCLASS
 */
UCLASS()
class MYBLANKPROGRAM_API UMyBlankObject : public UObject
{
	GENERATED_BODY()

public:
	static void ExampleUObject();
	static void ExampleReflection();
	static void ExampleSerialization2Json();
	static void ExampleStructFieldProperty();

	// friend FArchive& operator <<(FArchive& Ar, UMyBlankObject& MyObj);

	float MemVarCPP;

public:
	bool SaveData();
	void LoadData();
	void Dump();

	bool SaveData2();
	void LoadData2();

	void Serialize2Json(FStructuredArchive& Ar);

	UPROPERTY()
	float Health;

	UPROPERTY()
	int32 Ammo;

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FMyStruct StructVar;

	UFUNCTION(BlueprintCallable, Category=MyBlank)
	void Hello(FString Value);

	/** C++实现、蓝图调用 */
	UFUNCTION(BlueprintCallable, Category=MyBlank)
	void Hello_Callable();

	//** C++实现默认版、蓝图可以重载实现 */
	UFUNCTION(BlueprintNativeEvent, Category=MyBlank)
	void Hello_Native();

	/** C++不实现、蓝图实现 */
	UFUNCTION(BlueprintImplementableEvent, Category=MyBlank)
	void Hello_BPImplement();
};

// Invoke Function Template
template <typename ... TReturns, typename ... TArgs>
inline void InvokeFunction(UClass* objClass, UObject* obj, UFunction* func, TTuple<TReturns...>& outParams,
                           TArgs&&... args)
{
	objClass = obj != nullptr ? obj->GetClass() : objClass;
	UObject* context = obj != nullptr ? obj : objClass;
	uint8* outPramsBuffer = (uint8*)&outParams;

	if (func->HasAnyFunctionFlags(FUNC_Native)) //quick path for c++ functions
	{
		TTuple<TArgs..., TReturns...> params(Forward<TArgs>(args)..., TReturns()...);
		context->ProcessEvent(func, &params);
		//copy back out params
		for (TFieldIterator<FProperty> i(func); i; ++i)
		{
			FProperty* prop = *i;
			if (prop->PropertyFlags & CPF_OutParm)
			{
				void* propBuffer = prop->ContainerPtrToValuePtr<void*>(&params);
				prop->CopyCompleteValue(outPramsBuffer, propBuffer);
				outPramsBuffer += prop->GetSize();
			}
		}
		return;
	}

	TTuple<TArgs...> inParams(Forward<TArgs>(args)...);
	void* funcPramsBuffer = (uint8*)FMemory_Alloca(func->ParmsSize);
	uint8* inPramsBuffer = (uint8*)&inParams;

	for (TFieldIterator<FProperty> i(func); i; ++i)
	{
		FProperty* prop = *i;
		if (prop->GetFName().ToString().StartsWith("__"))
		{
			//ignore private param like __WolrdContext of function in blueprint funcion library
			continue;
		}
		void* propBuffer = prop->ContainerPtrToValuePtr<void*>(funcPramsBuffer);
		if (prop->PropertyFlags & CPF_OutParm)
		{
			prop->CopyCompleteValue(propBuffer, outPramsBuffer);
			outPramsBuffer += prop->GetSize();
		}
		else if (prop->PropertyFlags & CPF_Parm)
		{
			prop->CopyCompleteValue(propBuffer, inPramsBuffer);
			inPramsBuffer += prop->GetSize();
		}
	}

	context->ProcessEvent(func, funcPramsBuffer); //call function
	outPramsBuffer = (uint8*)&outParams; //reset to begin

	//copy back out params
	for (TFieldIterator<FProperty> i(func); i; ++i)
	{
		FProperty* prop = *i;
		if (prop->PropertyFlags & CPF_OutParm)
		{
			void* propBuffer = prop->ContainerPtrToValuePtr<void*>(funcPramsBuffer);
			prop->CopyCompleteValue(outPramsBuffer, propBuffer);
			outPramsBuffer += prop->GetSize();
		}
	}
}
