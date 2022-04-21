// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreUObject/Public/UObject/Object.h"
#include "MyBlankObject.h"
#include "MyInterface.h"
#include "MyBlankDerivedObject.generated.h"

/**
 * 
 */
UCLASS()
class MYBLANKPROGRAM_API UMyBlankDerivedObject : public UMyBlankObject, public IMyInterface
{
	GENERATED_BODY()

public:
	UPROPERTY()
	float DerivedValue;

	UFUNCTION()
	void HelloDerived()
	{
	}
};
