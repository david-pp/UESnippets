// Copyright Epic Games, Inc. All Rights Reserved.

#include <iostream>
#include "HelloUECpp.h"
#include "RequiredProgramMainCPPInclude.h"

IMPLEMENT_APPLICATION(HelloUECpp, "HelloUECpp");

int main(int argc, const char* argv[])
{
	FVector V1(1, 0, 0);
	FVector V2(0, 1, 0);

	float Value = V1 | V2; // Dot Product
	Value += 1;

	std::cout << "Hello UE C++! V1 * V2 = " << Value << std::endl;
}
