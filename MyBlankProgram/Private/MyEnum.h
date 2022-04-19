// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyEnum.generated.h"

/** 
 *  UENUM
 */
UENUM()
namespace EMyEnumOld
{
	enum Type
	{
		Type1,
		Type2
	};
}


UENUM(BlueprintType, meta=(EnumDisplayName="GetMyEnumDisplayName"))
enum class EMyEnum : uint8
{
	MyEnum_1 UMETA(DisplayName = "MyEnum_Item1"),
	MyEnum_2 UMETA(DisplayName = "MyEnum_Item2"),
};


inline FText GetMyEnumDisplayName(int32 Val)
{
	EMyEnum MyEnum = EMyEnum(Val);
	switch (MyEnum)
	{
	case EMyEnum::MyEnum_1:
		return FText::FromString(TEXT("MyEnumItem1"));
	case EMyEnum::MyEnum_2:
		return FText::FromString(TEXT("MyEnumItem2"));
	default:
		return FText::FromString(TEXT("MyEnumInvalid"));
	}
}
