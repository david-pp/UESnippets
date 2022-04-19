// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreUObject/Public/UObject/Interface.h"
#include "MyInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UMyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MYBLANKPROGRAM_API IMyInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintImplementableEvent)
	void BPFunction() const;

	virtual void Hello() const
	{
	}
};
