// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreUObject/Public/UObject/Object.h"
#include "MyInterface.h"
#include "MyStruct.h"
#include "MyEnum.h"
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

	// friend FArchive& operator <<(FArchive& Ar, UMyBlankObject& MyObj);

public:
	bool SaveData();
	void LoadData();
	void Dump();

	bool SaveData2();
	void LoadData2();

	UPROPERTY()
	float Health;

	UPROPERTY()
	int32 Ammo;

	UPROPERTY()
	FVector Location;

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
