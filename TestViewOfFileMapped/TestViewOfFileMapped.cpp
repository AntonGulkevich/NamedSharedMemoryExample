// TestViewOfFileMapped.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

#define UBS_MEM_SIZE 0xffffffff
#define PAGE_SIZE 0x10000 //64Kb 0x40
#include <iostream>

bool writeData(const HANDLE hMapObject, LPBYTE data, DWORD size, DWORD offset);
bool readData(const HANDLE hMapObject, LPBYTE data, DWORD size, DWORD offset);

DWORD read_dword();
VOID write_dword();

FLOAT read_float();
VOID write_float();

bool writeData(const HANDLE hMapObject, LPBYTE data, DWORD size, DWORD offset)
{
	std::cout << "Write data " << std::endl;
	DWORD multiOffset = (offset / PAGE_SIZE)*PAGE_SIZE;
	DWORD deltaOffset = offset - multiOffset;
	LPBYTE lpvMem = static_cast<LPBYTE> (MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS, 0, multiOffset, size+ deltaOffset));
	if (lpvMem == nullptr)
	{
		std::cerr << "Could not map view of file: error code: " << GetLastError() << std::endl;
		return FALSE;
	}
	lpvMem += deltaOffset;
	while (size--)
		*lpvMem++ = *data++;
	UnmapViewOfFile(lpvMem);
	return TRUE;
}

bool readData(const HANDLE hMapObject, LPBYTE data, DWORD size, DWORD offset)
{
	std::cerr << "Read data" << std::endl;
	DWORD multiOffset = (offset / PAGE_SIZE)*PAGE_SIZE;
	DWORD deltaOffset = offset - multiOffset;
	LPBYTE lpvMem = static_cast<LPBYTE> (MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS, 0, multiOffset, size + deltaOffset));
	if (lpvMem == nullptr)
	{
		std::cerr << "Could not map view of file: error code: " << GetLastError() << std::endl;
		return FALSE;
	}
	lpvMem += deltaOffset;
	memcpy(data, lpvMem, size);
	UnmapViewOfFile(lpvMem);
	return TRUE;
}

void write_dword()
{
}

FLOAT read_float()
{
}

void write_float()
{
}

DWORD read_dword()
{
}

int main()
{
	HANDLE hMapObject = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, UBS_MEM_SIZE, TEXT("dllmemfilemap"));
	if (hMapObject == nullptr) {
		std::cerr << "Could not create file: error code: " << GetLastError() << std::endl;
		system("PAUSE");
		return FALSE;
	}
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	std::cout << "Allocation Granularity = " << systemInfo.dwAllocationGranularity << std::endl;

	BYTE testStr[] = "THIS IS TEST STRING 1234567890";
	int size = sizeof(testStr);
	LPBYTE test_buff = new BYTE[size];
	memcpy(test_buff,testStr, size);

	std::cout << "Default string: " << test_buff << std::endl;
	writeData(hMapObject, test_buff, size, 0);

	LPBYTE rdStr = new BYTE[size];
	readData(hMapObject, rdStr, size, 0);
	std::cout << "Read string: " << rdStr << std::endl;
	
	system("PAUSE");

	CloseHandle(hMapObject);
	delete[] test_buff;
	delete[] rdStr;

    return 0;
}

