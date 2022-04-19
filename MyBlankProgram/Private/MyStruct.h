// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyStruct.generated.h"
/**
 * USTRUCT
 */
USTRUCT(BlueprintType)
struct FMyStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float Value;
};
