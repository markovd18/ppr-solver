#include <iostream>
#include "test/test.h"

int wmain(int argc, wchar_t **argv)
{

	std::wcout << L"vystup programu" << std::endl;

	const int i = 15;

	for (size_t i = 0; i < 68; i++)
	{
		std::wcout << i << std::endl;
	}

	test();
}