// Copyright Epic Games, Inc. All Rights Reserved.

#pragma warning(disable: 4668) 

#include <iostream>
#include "HelloUECpp.h"
#include "RequiredProgramMainCPPInclude.h"


IMPLEMENT_APPLICATION(HelloUECpp, "HelloUECpp");


void Example_Logging()
{
	UE_LOG(LogTemp, Display, TEXT("日志..."));
	UE_LOG(LogTemp, Warning, TEXT("日志..."));
}

int main(int argc, const char* argv[])
{
	FVector V1(1, 0, 0);
	FVector V2(0, 1, 0);

	float Value = V1 | V2; // Dot Product
	Value += 1;

	std::cout << "Hello UE C++! V1 * V2 = " << Value << std::endl;
	
	Example_Logging();
}
